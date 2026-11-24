import contextlib
import io
import json
import os
import tempfile
import time
from pathlib import Path

import matplotlib
import numpy as np

matplotlib.use("Agg")
from rdca_fit import model_counts
from scipy.special import ndtr
from tpcqa_conditions import write_csv

notebook = json.loads(Path(__file__).with_name("TPC_rDCA_AllRuns_Robust.ipynb").read_text())
rng = np.random.default_rng(88)
with tempfile.TemporaryDirectory() as tmp:
    data = Path(tmp) / "qa-direct-all"
    data.mkdir()
    records = []
    for group, run in [
        ("run3pp/ana585_2026p006_v001", 81558),
        ("run3oo/ana585_2026p006_v001", 82688),
    ]:
        maps = []
        arrays = {}
        charges = []
        for window in ["pt_0p5_1", "pt_1_2", "pt_2_4"]:
            for side in ["south", "north"]:
                for charge in ["pos", "neg"]:
                    charges.append(
                        dict(window=window, side=side, charge=charge, count=100000, variance=100000)
                    )
                    for axis in ["phi", "eta"]:
                        xedges = (
                            np.linspace(-np.pi, np.pi, 25)
                            if axis == "phi"
                            else np.linspace(-1.2, 1.2, 13)
                        )
                        x = (xedges[:-1] + xedges[1:]) / 2
                        edges = np.linspace(-10, 10, 101)
                        mean = (
                            0.1
                            + 0.12 * np.cos(x)
                            + (1 if charge == "pos" else -1) * (0.06 + 0.02 * np.sin(x))
                        )
                        counts = np.array(
                            [
                                rng.poisson(
                                    model_counts([np.log(4000), m, np.log(0.25), 2.0], edges)
                                )
                                for m in mean
                            ],
                            float,
                        )
                        key = f"m{len(maps):03d}"
                        maps.append(
                            dict(key=key, window=window, side=side, charge=charge, axis=axis)
                        )
                        for name, value in dict(
                            counts=counts, variance=counts, xedges=xedges, dcaedges=edges
                        ).items():
                            arrays[key + "_" + name] = value
        timing = []
        for side in ("south", "north"):
            key = "timing_" + side
            edges = np.arange(-0.5, 401.0, 1.0)
            x = (edges[:-1] + edges[1:]) / 2
            counts = 20 + 1000 * ndtr((x - 8.5) / 1.2) * ndtr((250.0 - x) / 2.0)
            arrays.update(
                {key + "_xedges": edges, key + "_counts": counts, key + "_variance": counts}
            )
            timing.append(
                dict(side=side, key=key, status="complete", source="cleaned_ADC_unpacked_time")
            )
        filename = f"{run}.npz"
        np.savez_compressed(data / filename, **arrays)
        records.append(
            dict(
                group=group,
                run=run,
                source_segments=100,
                maps=maps,
                file=filename,
                dedx_status="no_offline_cut",
                issues=[],
                charge_counts=charges,
                timing=timing,
            )
        )
    conditions = []
    references = []
    for record in records:
        for side in ("south", "north"):
            conditions.append(
                dict(
                    group=record["group"],
                    run=record["run"],
                    side=side,
                    cdb_tag="test",
                    keff_base=1.1,
                    keff_cdb=1.1,
                    keff_from_igem=1.2,
                    keff_reference=1.1,
                    keff_kind="cdb_base_not_verified_applied",
                    keff_applied=None,
                    gem_current_uA=2.0,
                    gem_current_rms_uA=0.2,
                    conditions_status="complete_snapshot",
                    current_scope="full_payload_not_selected_segments",
                )
            )
            references.append(
                dict(
                    group=record["group"],
                    run=record["run"],
                    side=side,
                    velocity_cm_us=7.4,
                    t0_bins=8.0,
                    z_pad_cm=102.5,
                    cm_halfwidth_cm=0.175,
                    source="synthetic PHGarfield test",
                )
            )
    for row in conditions:
        if row["group"].startswith("run3oo"):
            row.update(
                keff_base=None,
                keff_cdb=None,
                keff_reference=1.2,
                keff_kind="derived_igem_not_verified_applied",
            )
    write_csv(data / "conditions_snapshot.csv", conditions)
    write_csv(data / "phgarfield_reference.csv", references)
    (data / "manifest.json").write_text(
        json.dumps(
            dict(
                schema=1,
                fit_performed=False,
                input_mode="production_qa_histograms",
                runs=records,
                conditions=dict(
                    status="exported",
                    file="conditions_snapshot.csv",
                    phgarfield_file="phgarfield_reference.csv",
                ),
            )
        )
    )
    (data / "SUCCESS").touch()
    os.chdir(tmp)
    ns = {}
    started = time.monotonic()
    with contextlib.redirect_stdout(io.StringIO()):
        for i, cell in enumerate(notebook["cells"]):
            if cell["cell_type"] == "code":
                exec(compile("".join(cell["source"]), f"cell_{i}", "exec"), ns)
                if i == 1:
                    ns["BOOTSTRAP_REPEATS"] = 30
                    ns["DATA_DIR"] = Path("qa-direct-all")
    table = ns["RUN_SUMMARY"]
    assert len(table) == 12, len(table)
    assert (table.status == "estimated").all(), table.status
    assert (table.common_phi_bins >= 8).all()
    np.testing.assert_allclose(table.full_difference_mm, 2 * table.odd_half_mm)
    np.testing.assert_allclose(table.delta_common_odd_half_mm, 0)
    assert (Path("rdca_odd_all_runs") / "rdca_odd_robust_per_run.csv").is_file()
    assert len(ns["TIMING_SUMMARY"]) == 4
    assert (ns["TIMING_SUMMARY"].status == "estimated").all()
    assert ns["TIMING_SUMMARY"].velocity_ratio.notna().all()
    assert table.keff_reference.notna().all()
    assert table.loc[table.group.str.startswith("run3oo"), "keff_cdb"].isna().all()
    assert len(ns["CHARGE_SUMMARY"]) == 12
    print(
        f"All notebook code cells passed on synthetic two-system data: 12 summaries, {time.monotonic() - started:.1f}s"
    )
