#!/usr/bin/env python3
"""Read production QA HIST files directly and export small unfitted arrays.

One code path for pilot and full runs. No old inventory, caches, batch record,
combined summary or compiled calibration-QA library is used.
"""

import argparse
import hashlib
import heapq
import json
import multiprocessing
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

import numpy as np
from tpcqa_arrays import charge_balance, project_file
from tpcqa_conditions import write_csv
from tpcqa_timing import time_spectra

PATTERN = re.compile(
    r"HIST_(DST_TRKR_SEED|DST_TRKR_TRACKS)_(run3(?:pp|oo|auau))_([A-Za-z0-9_.]+)-(\d{8})-(\d+)\.root"
)


def digest(value):
    return hashlib.sha256(
        json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    ).hexdigest()


def atomic_json(path, value):
    temporary = path.with_name(path.name + ".partial")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)


def identity(path, segment):
    resolved = Path(path).resolve(strict=True)
    if not resolved.is_file():
        raise ValueError(f"Not a regular histogram file: {resolved}")
    stat = resolved.stat()
    return dict(
        path=str(resolved), segment=int(segment), size=stat.st_size, mtime_ns=stat.st_mtime_ns
    )


def check_unchanged(item):
    stat = Path(item["path"]).stat()
    if (stat.st_size, stat.st_mtime_ns) != (item["size"], item["mtime_ns"]):
        raise ValueError(f"Input changed during export: {item['path']}")


def discover(sources, selections, all_runs, max_files):
    """Stream directory names; retain only the lowest max_files segment IDs.

    Only selected files are stat'ed and opened. Both numeric run directories
    and historical run_XXXXXXXX_XXXXXXXX directories are supported.
    """
    heaps, seen, counts = {}, {}, {}
    source_info = []
    systems = set()
    for spec in sources:
        system, text = spec.split("=", 1)
        if system not in ("run3pp", "run3oo", "run3auau") or system in systems:
            raise ValueError("Use one explicit product source per system")
        systems.add(system)
        source = Path(text).expanduser().resolve(strict=True)
        if not source.is_dir() or source.name not in ("DST_TRKR_SEED", "DST_TRKR_TRACKS"):
            raise ValueError("Supply the explicit DST_TRKR_SEED or DST_TRKR_TRACKS directory")
        wanted = {run for sys, run in selections if sys == system}
        if not all_runs and not wanted:
            raise ValueError(f"No --select supplied for {system}")
        source_info.append(dict(system=system, path=str(source)))
        folders = []
        with os.scandir(source) as entries:
            for entry in entries:
                match = re.fullmatch(r"run_(\d{8})_(\d{8})", entry.name)
                if not entry.name.isdecimal() and not match:
                    continue
                if not entry.is_dir():
                    continue
                if not all_runs:
                    if entry.name.isdecimal() and int(entry.name) not in wanted:
                        continue
                    if match and not any(int(match[1]) <= run <= int(match[2]) for run in wanted):
                        continue
                folders.append(Path(entry.path))
        print(
            f"{system}: scanning filenames in {len(folders)} run directories; cap={max_files}",
            flush=True,
        )
        for folder in sorted(folders):
            hist = folder / "hist"
            if not hist.is_dir():
                continue
            with os.scandir(hist) as entries:
                for entry in entries:
                    match = PATTERN.fullmatch(entry.name)
                    if not match:
                        continue
                    product, actual_system, tag, run, segment = match.groups()
                    run, segment = int(run), int(segment)
                    if product != source.name or actual_system != system:
                        raise ValueError(f"Product/system differs from source: {entry.path}")
                    if folder.name.isdecimal() and int(folder.name) != run:
                        raise ValueError(f"Run number differs from directory: {entry.path}")
                    if not all_runs and run not in wanted:
                        continue
                    key = (system + "/" + tag, run)
                    observed = seen.setdefault(key, set())
                    if segment in observed:
                        raise ValueError(
                            f"Duplicate segment {key}/{segment}; select one source location"
                        )
                    observed.add(segment)
                    counts[key] = counts.get(key, 0) + 1
                    heap = heaps.setdefault(key, [])
                    candidate = (-segment, entry.path)
                    if len(heap) < max_files:
                        heapq.heappush(heap, candidate)
                    elif segment < -heap[0][0]:
                        heapq.heapreplace(heap, candidate)
    if set(sys for sys, _ in selections) - systems:
        raise ValueError("A selected system has no --source")
    for system, run in selections:
        matches = [key for key in heaps if key[0].split("/")[0] == system and key[1] == run]
        if len(matches) != 1:
            raise ValueError(f"Missing or ambiguous selected run: {system}/{run}")
    records = []
    physical_inputs = {}
    for (group, run), heap in sorted(heaps.items()):
        files = [
            identity(path, -minus_segment) for minus_segment, path in sorted(heap, reverse=True)
        ]
        canonical = [f["path"] for f in files]
        if len(canonical) != len(set(canonical)):
            raise ValueError("Different segment names resolve to the same physical file")
        for path in canonical:
            if path in physical_inputs:
                raise ValueError(f"One input assigned to multiple run/groups: {path}")
            physical_inputs[path] = (group, run)
        records.append(
            dict(
                group=group,
                run=run,
                files=files,
                source_segments=len(files),
                discovered_segments=counts[(group, run)],
            )
        )
        print(
            f"{group}/{run}: selected {len(files)}/{counts[(group, run)]} files, "
            f"segments {files[0]['segment']}..{files[-1]['segment']}",
            flush=True,
        )
    if not records:
        raise ValueError("No QA files found")
    return records, source_info


def export_run(record, config, out_text, resume):
    started = time.monotonic()
    out = Path(out_text)
    stem = record["group"].replace("/", "__") + f"__{record['run']}"
    target = out / (stem + ".npz")
    status = out / (stem + ".json")
    fingerprint = digest(dict(record=record, config=config))
    if resume and status.is_file() and target.is_file():
        old = json.loads(status.read_text())
        if (
            old.get("fingerprint") == fingerprint
            and old.get("output_sha256") == hashlib.sha256(target.read_bytes()).hexdigest()
        ):
            print(
                f"{record['group']}/{record['run']}: reused verified direct-QA export", flush=True
            )
            return old
    # Each process has independent ROOT state. No fitting or graphics libraries
    # are imported by the exporter itself.
    import ROOT

    ROOT.gROOT.SetBatch(True)
    ROOT.TH1.AddDirectory(True)
    sums = maps = charges = schema = None
    for number, item in enumerate(record["files"], 1):
        check_unchanged(item)
        root = ROOT.TFile.Open(item["path"], "READ")
        if not root or root.IsZombie() or root.TestBit(ROOT.TFile.kRecovered):
            raise ValueError(f"Unusable QA ROOT file: {item['path']}")
        try:
            arrays, current_maps, current_charges, current_schema = project_file(
                root, config["windows"], config["prefix"], config["phi_group"]
            )
            time_arrays, current_timing = time_spectra(root)
            arrays.update(time_arrays)
        finally:
            root.Close()
        check_unchanged(item)
        if sums is None:
            sums, maps, charges, schema = arrays, current_maps, current_charges, current_schema
            timing = current_timing
        else:
            if current_schema != schema or current_timing != timing or set(arrays) != set(sums):
                raise ValueError(f"Mixed QA axes/selection/schema: {item['path']}")
            for key, value in arrays.items():
                if key.endswith(("_counts", "_variance")):
                    sums[key] += value
                elif not np.array_equal(sums[key], value):
                    raise ValueError(f"Projected binning differs: {item['path']}")
            for old, new in zip(maps, current_maps, strict=True):
                for field in ("dca_underflow", "dca_overflow"):
                    old[field] += new[field]
            for old, new in zip(charges, current_charges, strict=True):
                old["count"] += new["count"]
                old["variance"] += new["variance"]
        if number == 1 or number % 10 == 0 or number == len(record["files"]):
            print(
                f"{record['group']}/{record['run']}: read {number}/{len(record['files'])} QA files; "
                f"{time.monotonic() - started:.1f}s",
                flush=True,
            )
    for item in record["files"]:
        check_unchanged(item)
    with target.with_suffix(".partial.npz").open("wb") as stream:
        np.savez_compressed(stream, **sums)
    target.with_suffix(".partial.npz").replace(target)
    result = dict(
        group=record["group"],
        run=record["run"],
        file=target.name,
        source_segments=record["source_segments"],
        discovered_segments=record["discovered_segments"],
        source_files=record["files"],
        input_list_sha256=digest(record["files"]),
        fingerprint=fingerprint,
        output_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
        maps=maps,
        charge_counts=charges,
        timing=timing,
        issues=[f"timing {t['side']}: {t['status']}" for t in timing if t["status"] != "complete"],
        dedx_status="no_additional_offline_cut",
        elapsed_seconds=time.monotonic() - started,
    )
    atomic_json(status, result)
    print(
        "Timing availability: " + ", ".join(t["side"] + "=" + t["status"] for t in timing),
        flush=True,
    )
    print(
        f"{record['group']}/{record['run']}: saved {target.stat().st_size / 1024:.0f} KiB; "
        f"{result['elapsed_seconds']:.1f}s",
        flush=True,
    )
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", action="append", required=True, metavar="SYSTEM=PATH")
    parser.add_argument("--select", action="append", default=[], metavar="SYSTEM=RUN")
    parser.add_argument("--all-runs", action="store_true")
    parser.add_argument("--max-files", type=int, default=100, help="Per-run cap, 1..100")
    parser.add_argument("--pt-windows", default="0.5:1,1:2,2:4")
    parser.add_argument("--phi-group", type=int, default=3)
    parser.add_argument("--prefix", default="h_TpcSeedsQA_")
    parser.add_argument("--jobs", type=int, default=2)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--resume", action="store_true")
    parser.add_argument(
        "--cdb-tag", help="Fetch a fresh current-CDB snapshot per run; explicit global tag"
    )
    parser.add_argument(
        "--reference-field",
        type=float,
        help="Also export PHGarfield full-run P/T baseline [V/cm], using installed TpcCalibrationQA",
    )
    parser.add_argument(
        "--keff-fallback",
        choices=("none", "igem"),
        default="none",
        help="Explicit derived comparison when no CDB kEff payload exists",
    )
    args = parser.parse_args()
    if args.reference_field is not None and (
        not args.cdb_tag or not np.isfinite(args.reference_field) or args.reference_field <= 0
    ):
        parser.error("--reference-field requires --cdb-tag and a positive finite value")
    if args.cdb_tag is not None and not args.cdb_tag.strip():
        parser.error("--cdb-tag cannot be empty")
    if bool(args.select) == bool(args.all_runs):
        parser.error("Choose --select (repeatable) OR --all-runs")
    if not (1 <= args.max_files <= 100 and 1 <= args.jobs <= 4 and args.phi_group > 0):
        parser.error("Require max-files 1..100, jobs 1..4, and positive grouping")
    selections = []
    for text in args.select:
        system, run = text.split("=", 1)
        selections.append((system, int(run)))
    if len(selections) != len(set(selections)):
        parser.error("Duplicate selection")
    windows = []
    for text in args.pt_windows.split(","):
        low, high = map(float, text.split(":"))
        if not (np.isfinite(low) and np.isfinite(high) and 0 < low < high):
            parser.error("Invalid pT window")
        windows.append((f"pt_{low:g}_{high:g}".replace(".", "p"), low, high))
    if len(windows) != len(set(windows)):
        parser.error("Duplicate pT window")
    config = dict(
        windows=windows,
        phi_group=args.phi_group,
        prefix=args.prefix,
        max_files=args.max_files,
        reader_schema=1,
        code_sha256=digest(
            {
                Path(__file__).name: Path(__file__).read_text(),
                "tpcqa_arrays.py": Path(__file__).with_name("tpcqa_arrays.py").read_text(),
                "tpcqa_timing.py": Path(__file__).with_name("tpcqa_timing.py").read_text(),
            }
        ),
    )
    started = time.monotonic()
    if args.cdb_tag:
        print("Checking conditions/PHGarfield dependencies before reading QA files", flush=True)
        command = [
            sys.executable,
            str(Path(__file__).with_name("tpcqa_conditions.py")),
            "--check-environment",
        ]
        if args.reference_field is not None:
            command += ["--reference-field", str(args.reference_field)]
        subprocess.run(command, check=True)
    records, sources = discover(args.source, selections, args.all_runs, args.max_files)
    out = args.out.expanduser().resolve()
    out.mkdir(parents=True, exist_ok=args.resume)
    if (out / "SUCCESS").exists():
        (out / "SUCCESS").unlink()
    atomic_json(out / "inputs.json", dict(sources=sources, configuration=config, runs=records))
    complete, failed = [], []
    if args.jobs == 1:
        for record in records:
            try:
                complete.append(export_run(record, config, str(out), args.resume))
            except Exception as error:
                failed.append(dict(group=record["group"], run=record["run"], error=str(error)))
                print("FAILED:", failed[-1], flush=True)
    else:
        with ProcessPoolExecutor(
            max_workers=args.jobs, mp_context=multiprocessing.get_context("spawn")
        ) as pool:
            pending = {
                pool.submit(export_run, record, config, str(out), args.resume): record
                for record in records
            }
            for future in as_completed(pending):
                record = pending[future]
                try:
                    complete.append(future.result())
                except Exception as error:
                    failed.append(dict(group=record["group"], run=record["run"], error=str(error)))
                    print("FAILED:", failed[-1], flush=True)
    manifest = dict(
        schema=1,
        kind="unfitted_tpcqa_arrays",
        input_mode="production_qa_histograms",
        fit_performed=False,
        sources=sources,
        configuration=config,
        runs=sorted(complete, key=lambda r: (r["group"], r["run"])),
        failed=failed,
        elapsed_seconds=time.monotonic() - started,
        note="Direct production QA input; no additional dE/dx cut; no historical cache required.",
    )
    # Charges are derived from the same selected QA files, without fitting.
    charge_rows = []
    for record in manifest["runs"]:
        for item in record["charge_counts"]:
            charge_rows.append(dict(group=record["group"], run=record["run"], **item))
    if charge_rows:
        write_csv(out / "charge_counts.csv", charge_rows)
        write_csv(out / "charge_balance.csv", charge_balance(manifest["runs"]))
    manifest["conditions"] = dict(status="not_requested", historical_applied_verified=False)
    if args.cdb_tag and not failed:
        # Each run gets a new process: CDBInterface caches URLs and must not be reused across runs.
        from concurrent.futures import ThreadPoolExecutor

        directory = out / "conditions"
        directory.mkdir(exist_ok=True)

        def query(record):
            target = directory / (record["group"].replace("/", "_") + f"_{record['run']}.json")
            if target.exists():
                target.unlink()
            command = [
                sys.executable,
                str(Path(__file__).with_name("tpcqa_conditions.py")),
                "--group",
                record["group"],
                "--run",
                str(record["run"]),
                "--cdb-tag",
                args.cdb_tag,
                "--out",
                str(target),
                "--keff-fallback",
                args.keff_fallback,
            ]
            if args.reference_field is not None:
                command += ["--reference-field", str(args.reference_field)]
            result = subprocess.run(command, text=True, capture_output=True)
            log = target.with_suffix(".log")
            log.write_text(result.stdout + "\n" + result.stderr)
            if result.returncode:
                raise RuntimeError(f"CDB snapshot failed; inspect {log}")
            return json.loads(target.read_text())

        snapshots = []
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            pending = {pool.submit(query, r): r for r in manifest["runs"]}
            for future in as_completed(pending):
                record = pending[future]
                try:
                    snapshots.append(future.result())
                    print(
                        f"{record['group']}/{record['run']}: fresh CDB snapshot exported",
                        flush=True,
                    )
                except Exception as error:
                    failed.append(dict(group=record["group"], run=record["run"], error=str(error)))
                    print("FAILED:", failed[-1], flush=True)
        rows = sorted(
            [r for snapshot in snapshots for r in snapshot["rows"]],
            key=lambda r: (r["group"], r["run"], r["side"]),
        )
        phgarfield = sorted(
            [r for snapshot in snapshots for r in snapshot.get("phgarfield", [])],
            key=lambda r: (r["group"], int(r["run"]), r["side"]),
        )
        if phgarfield:
            write_csv(out / "phgarfield_reference.csv", phgarfield)
        if rows:
            write_csv(out / "conditions_snapshot.csv", rows)
        manifest["conditions"] = dict(
            status="exported" if not failed else "failed",
            cdb_tag=args.cdb_tag,
            file="conditions_snapshot.csv",
            keff_fallback=args.keff_fallback,
            derived_igem_sides=sum(
                r["keff_kind"] == "derived_igem_not_verified_applied" for r in rows
            ),
            phgarfield_file="phgarfield_reference.csv" if phgarfield else None,
            reference_field_Vcm=args.reference_field,
            incomplete_sides=sum(r["conditions_status"] != "complete_snapshot" for r in rows),
            historical_applied_verified=False,
            scope="current_CDB_full_payload",
        )
    manifest["elapsed_seconds"] = time.monotonic() - started
    atomic_json(out / "manifest.json", manifest)
    if failed:
        raise RuntimeError(
            f"{len(failed)} runs failed; inspect manifest.json. Retry with --resume after fixing inputs."
        )
    (out / "SUCCESS").write_text("Direct production QA arrays exported\n")
    print(
        f"Ready: {out}; {len(complete)} runs; elapsed {manifest['elapsed_seconds']:.1f}s",
        flush=True,
    )


if __name__ == "__main__":
    main()
