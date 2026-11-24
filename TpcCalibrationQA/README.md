# TpcCalibrationQA

Compact run-by-run TPC diagnostics from production QA histograms: charge-odd
rDCA, charge balance by pT, drift velocity/t0 and PHGarfield comparisons.
Python sums only necessary histograms on SDCC, with a 100-segment cap per run.
A self-contained laptop notebook performs fitting and creates small CSV tables.
The native library only supplies the PHGarfield P/T reference integrator.

## Fresh build and two-run validation

Use the same sPHENIX environment as the successful earlier production test.
The clean archive should be extracted into a new directory, so removed legacy
files cannot survive an in-place unzip. Existing work and installation stay intact.

```bash
export TPCQA_WORK=/sphenix/user/mitrankov/tpcqa-migration-20261123
unzip -o /sphenix/user/mitrankov/TpcCalibrationQA_clean.zip -d "$TPCQA_WORK/clean-src"
export TPCQA_SRC="$TPCQA_WORK/clean-src/TrackingAnalysis/TpcCalibrationQA"
export TPCQA_CDB_TAG=2026p006
export TPCQA_REFERENCE_FIELD=376.85
bash "$TPCQA_SRC/tools/build_and_pilot.sh"
```

This builds under `clean-build`, installs under `clean-install`, runs the native
numerical check and rereads the two original QA run samples into
`clean-validation/qa-direct-pilot.tgz`. Field 376.85 V/cm is the explicitly chosen
comparison baseline; the production logs establish tag 2026p006 but not this field.
`TPCQA_CLEAN_INSTALL` can override the isolated installation prefix.

Then run full native lint with the actual installed dependency headers:

```bash
export TPCQA_INSTALL="$TPCQA_WORK/clean-install"
bash "$TPCQA_SRC/tools/validate_sdcc.sh" "$TPCQA_WORK/clean-build"
```

The script requires clang-format and clang-tidy on PATH and fails if either is
missing. It creates a compile database using root-config, checks every native
source and test, and runs make check. Missing tools or dependency parse errors
are not treated as a clean check. The build uses -Wall -Wextra -Wshadow -Werror.

Transfer/extract the pilot archive next to the notebook in `diagnostics/`.
Set DATA_DIR to the extracted pilot path in its first code cell, restart the
kernel and Run All. Python and notebook lint can be checked with Ruff:

```bash
ruff check "$TPCQA_SRC"
ruff format --check "$TPCQA_SRC"
python3 -m unittest discover -s "$TPCQA_SRC/diagnostics" -p 'test_*.py' -v
python3 "$TPCQA_SRC/diagnostics/check_notebook.py"
```

The numerical notebook tests need NumPy/SciPy/pandas/Matplotlib/IPython and may
run on the laptop. The SDCC histogram exporter needs PyROOT and NumPy.

## Full run after the pilot

```bash
TPCQA_WORK="$TPCQA_WORK/clean-validation" \
TPCQA_INSTALL="$TPCQA_WORK/clean-install" \
bash "$TPCQA_SRC/diagnostics/run_sdcc.sh" all
```

The archive is `clean-validation/qa-direct-all.tgz`. The same source paths,
three pT windows, 100-segment cap and two-worker default apply. Change the
explicit source variables for future production. See diagnostics/README.md
for output schemas, histogram selections and timing coordinate settings.

## kEff source and scope

Current-CDB values remain in `keff_cdb` and the compatibility column `keff_base`.
`keff_from_igem` uses the full-payload R1 GEM-current recipe:

    1.55 * (0.185 + 0.815 * Igem / Iref)

Iref is 3.36138 uA South and 3.20357 uA North. `keff_reference` prefers a valid
CDB coefficient, otherwise uses the explicitly enabled recipe only when no CDB
coefficient payload exists and current coverage is complete and positive.
The notebook distinguishes derived points with triangular markers and exports
`keff_kind`, recipe constants and upstream source SHA. It does not recover an
O+O manual coefficient file, historical overrides or segment multipliers.
`keff_applied` remains empty. Invalid present payloads are not silently replaced.

## Removed legacy path

Only three C++ sources remain: the PHGarfield reference driver, integrator and
run-average conditions initializer. The old C++ fit engine, histogram collector,
large combined summary writer, old command runner, prototype exporters/notebook
and generated fixtures are excluded. ROOT-independent numerical helpers and
focused tests remain. No production event-QA module is added or modified here.

The notebook's fit threshold stays at 1000 with quality flags. QA observables
are not automatically converted into calibration corrections. Raw-time offsets,
edge windows and reference geometry are explicit, and missing inputs remain
unavailable. No new dE/dx cut is claimed.
