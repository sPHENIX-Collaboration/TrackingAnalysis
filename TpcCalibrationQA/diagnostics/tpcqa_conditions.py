#!/usr/bin/env python3
"""Current-CDB snapshot in a fresh process per run; never historical applied values."""

import argparse
import csv
import json
from datetime import datetime, timezone
from pathlib import Path

import numpy as np

R1_CHANNELS = {
    "south": ("S_01_R1_G4_IMon", "S_04_R1_G4_IMon", "S_05_R1_G4_IMon", "S_10_R1_G4_IMon"),
    "north": tuple(f"N_{sector:02d}_R1_G4_IMon" for sector in (3, 4, 6, 7, 8, 10, 11, 12)),
}


GEM_KEFF_RECIPE = dict(
    reference_keff=1.55,
    offset=0.185,
    slope=0.815,
    south_reference_current_uA=3.36138,
    north_reference_current_uA=3.20357,
)


def gem_current_keff(current, side):
    """PHGarfield default full-run R1 recipe; not a historical override recovery."""
    if current is None or not np.isfinite(current) or current <= 0:
        return None
    recipe = GEM_KEFF_RECIPE
    reference = recipe[side + "_reference_current_uA"]
    return recipe["reference_keff"] * (recipe["offset"] + recipe["slope"] * current / reference)


def entries(mapping):
    if hasattr(mapping, "items"):
        return dict(mapping.items())
    return {pair.first: pair.second for pair in mapping}


def summarize(values):
    good = np.asarray([v for v in values if np.isfinite(v)], float)
    return dict(
        valid_samples=len(good),
        mean=float(good.mean()) if len(good) else None,
        rms=float(good.std()) if len(good) else None,
        minimum=float(good.min()) if len(good) else None,
        maximum=float(good.max()) if len(good) else None,
    )


def rows_from_payload(
    group, run, tag, conditions, keff, conditions_url, keff_url, keff_fallback="none"
):
    """R1 channel median per BCO, then unweighted full-payload time-sample mean."""
    if keff_fallback not in ("none", "igem"):
        raise ValueError("Unknown kEff fallback")
    channels = {}
    if conditions is not None:
        for channel, fields in entries(conditions.GetUInt64EntryMap()).items():
            bco = int(entries(fields).get("gbco", 0))
            if bco in (0, 2**64 - 1):
                continue
            if bco in channels:
                raise ValueError(f"Duplicate conditions BCO: {bco}")
            channels[bco] = int(channel)
    coefficients = entries(keff.GetSingleFloatEntryMap()) if keff is not None else {}
    # Raw CDBTTree map keys contain the type prefix F (not the getter key).
    series = []
    for bco, channel in sorted(channels.items()):
        for side, names in R1_CHANNELS.items():
            values = np.array([conditions.GetFloatValue(channel, name) for name in names], float)
            current = float(np.median(values)) if np.isfinite(values).all() else None
            series.append(dict(group=group, run=run, side=side, bco=bco, gem_current_uA=current))
    rows = []
    for side, index in [("south", 0), ("north", 1)]:
        stats = summarize(
            [
                r["gem_current_uA"]
                for r in series
                if r["side"] == side and r["gem_current_uA"] is not None
            ]
        )
        coefficient = float(coefficients.get(f"Fkeffside{index}", float("nan")))
        coefficient = coefficient if np.isfinite(coefficient) and coefficient > 0 else None
        complete = bool(channels) and stats["valid_samples"] == len(channels)
        derived = gem_current_keff(stats["mean"], side) if complete else None
        reference = coefficient
        kind = "cdb_base_not_verified_applied" if coefficient is not None else "missing"
        # A present malformed payload must never be hidden by an Igem fallback.
        if keff_fallback == "igem" and not keff_url and keff is None and derived is not None:
            reference = derived
            kind = "derived_igem_not_verified_applied"
        elif coefficient is None and keff_url:
            kind = "invalid_cdb_payload_no_fallback"
        rows.append(
            dict(
                group=group,
                run=run,
                side=side,
                cdb_tag=tag,
                snapshot_utc=datetime.now(timezone.utc).isoformat(),
                keff_base=coefficient,
                keff_cdb=coefficient,
                keff_from_igem=derived,
                keff_reference=reference,
                keff_applied=None,
                keff_kind=kind,
                recipe_source_blob="db5a1f6ac4076abe4e6f184f9dcf6c40b7cdfecc",
                recipe_reference_keff=GEM_KEFF_RECIPE["reference_keff"],
                recipe_offset=GEM_KEFF_RECIPE["offset"],
                recipe_slope=GEM_KEFF_RECIPE["slope"],
                recipe_reference_current_uA=GEM_KEFF_RECIPE[side + "_reference_current_uA"],
                gem_current_uA=stats["mean"],
                gem_current_rms_uA=stats["rms"],
                gem_current_min_uA=stats["minimum"],
                gem_current_max_uA=stats["maximum"],
                conditions_samples=len(channels),
                valid_current_samples=stats["valid_samples"],
                conditions_status=("complete_snapshot" if reference is not None else "missing_keff")
                if complete
                else "missing_or_partial_current",
                current_scope="full_CDB_payload_not_selected_QA_segments",
                current_definition="unweighted_mean_of_R1_channel_medians",
                current_channels=";".join(R1_CHANNELS[side]),
                bco_first=min(channels) if channels else None,
                bco_last=max(channels) if channels else None,
                conditions_payload=conditions_url,
                keff_payload=keff_url,
                historical_applied_verified=False,
            )
        )
    return rows, series


def write_csv(path, rows):
    temporary = path.with_name(path.name + ".partial")
    with temporary.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    temporary.replace(path)


def root_environment(with_phgarfield=False):
    import ROOT

    ROOT.gROOT.SetBatch(True)
    for library in ("libphool.so", "libcdbobjects.so", "libffamodules.so"):
        if ROOT.gSystem.Load(library) < 0:
            raise RuntimeError(f"Cannot load {library}; use the sPHENIX environment")
    if not ROOT.gInterpreter.Declare(
        "#include <phool/recoConsts.h>\n#include <ffamodules/CDBInterface.h>\n#include <cdbobjects/CDBTTree.h>"
    ):
        raise RuntimeError("Cannot load CDB headers")
    if with_phgarfield:
        if ROOT.gSystem.Load("libtpccalibrationqa.so") < 0:
            raise RuntimeError(
                "PHGarfield comparison requires the existing TpcCalibrationQA installation"
            )
        if not ROOT.gInterpreter.Declare("#include <tpccalibrationqa/TpcConditionsReference.h>"):
            raise RuntimeError("Cannot load TpcConditionsReference header; set ROOT_INCLUDE_PATH")
    return ROOT


def snapshot(group, run, tag, output, reference_field=None, keff_fallback="none"):
    ROOT = root_environment(reference_field is not None)
    rc = ROOT.recoConsts.instance()
    rc.set_IntFlag("RUNNUMBER", run)
    rc.set_uint64Flag("TIMESTAMP", run)
    rc.set_StringFlag("CDB_GLOBALTAG", tag)
    cdb = ROOT.CDBInterface.instance()
    urls = {name: str(cdb.getUrl(name)) for name in ("TPC_CONDITIONS", "Tpc_PolyClusterizer_kEff")}
    payloads = {}
    for name, url in urls.items():
        payloads[name] = None
        if url:
            probe = ROOT.TFile.Open(url, "READ")
            if not probe or probe.IsZombie() or probe.TestBit(ROOT.TFile.kRecovered):
                raise ValueError(f"Unusable CDB payload: {url}")
            probe.Close()
            payloads[name] = ROOT.CDBTTree(url)
            payloads[name].LoadCalibrations()
    rows, _ = rows_from_payload(
        group,
        run,
        tag,
        payloads["TPC_CONDITIONS"],
        payloads["Tpc_PolyClusterizer_kEff"],
        urls["TPC_CONDITIONS"],
        urls["Tpc_PolyClusterizer_kEff"],
        keff_fallback,
    )
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(".partial.json")
    phgarfield = []
    if reference_field is not None:
        reference = ROOT.TpcConditionsReference()
        reference.set_cdb_tag(tag)
        reference.set_reference_field(reference_field)
        target = output.with_suffix(".phgarfield.csv")
        reference.export_phgarfield(run, group, str(target))
        with target.open() as stream:
            phgarfield = list(csv.DictReader(stream))
        if len(phgarfield) != 2 or {r["side"] for r in phgarfield} != {"0", "1"}:
            raise ValueError("PHGarfield reference must contain both sides")
        for row in phgarfield:
            row["side"] = "south" if row["side"] == "0" else "north"
    temporary.write_text(
        json.dumps(dict(rows=rows, phgarfield=phgarfield), indent=2, allow_nan=False) + "\n"
    )
    temporary.replace(output)
    print(
        f"{group}/{run}: CDB snapshot "
        + ", ".join("{}: {}".format(r["side"], r["conditions_status"]) for r in rows),
        flush=True,
    )


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--group")
    p.add_argument("--run", type=int)
    p.add_argument("--cdb-tag")
    p.add_argument("--out")
    p.add_argument("--check-environment", action="store_true")
    p.add_argument(
        "--reference-field",
        type=float,
        help="PHGarfield reference field [V/cm]; requires installed TpcCalibrationQA",
    )
    p.add_argument("--keff-fallback", choices=("none", "igem"), default="none")
    a = p.parse_args()
    if a.check_environment:
        root_environment(a.reference_field is not None)
        print("Conditions/PHGarfield dependencies available")
        raise SystemExit(0)
    if (
        not a.cdb_tag
        or not a.cdb_tag.strip()
        or not a.run
        or a.run <= 0
        or not a.group
        or not a.out
    ):
        p.error("Explicit group, CDB tag, positive run, and output required")
    if a.reference_field is not None and not (
        np.isfinite(a.reference_field) and a.reference_field > 0
    ):
        p.error("Positive PHGarfield reference field required")
    snapshot(a.group, a.run, a.cdb_tag, a.out, a.reference_field, a.keff_fallback)
