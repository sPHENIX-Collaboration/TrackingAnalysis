# Compact TPC QA: original production histograms → one portable dataset

One SDCC invocation reads the selected original QA files, sums only needed
histograms, exports charge balance by pT, queries current CDB conditions, and
runs the existing PHGarfield baseline integrator. The laptop notebook performs
rDCA fits, timing edge extraction and run comparisons. No event reconstruction,
large combined ROOT summary or per-slice PDF collection is needed.

## SDCC: test two runs first

Unzip this package under `/sphenix/user/mitrankov/tpcqa-migration-20261123`.
Use the normal sPHENIX environment with PyROOT/NumPy and the already-built
TpcCalibrationQA installation. PHGarfield uses its installed
`libtpccalibrationqa.so` and headers. If needed, retain `TPCQA_INSTALL` from the
previous build; the wrapper adds its library/include directories.

Choose the explicit global tag for the **current CDB comparison** and the intended
PHGarfield model reference field in V/cm. These are not inferred from the
production filename. Existing `TPCQA_CDB_TAG` and `TPCQA_REFERENCE_FIELD` variables
can be reused when they specify the intended comparison. Reference defaults inside
the installed integrator are P_ref=30.0968906 and T_ref=299.98646566 K,
t0=8 bins, clock=56.8 ns/bin and step=5 ns; the reference CSV records actual settings.

```bash
export TPCQA_WORK=/sphenix/user/mitrankov/tpcqa-migration-20261123
unzip -o /sphenix/user/mitrankov/TPC_QA_Direct_QuickFit.zip -d "$TPCQA_WORK"
: "${TPCQA_CDB_TAG:?Set the intended CDB comparison tag}"
: "${TPCQA_REFERENCE_FIELD:?Set the PHGarfield model reference field in V/cm}"
bash "$TPCQA_WORK/TPC_QA_Direct_QuickFit/run_sdcc.sh" pilot
```

The pilot uses pp 81558 and OO 82688, up to 100 segments each, two processes,
and pT windows 0.5–1, 1–2 and 2–4 GeV/c. Source paths default to
`/sphenix/data/data02/sphnxpro/production/run3{pp,oo}/physics/ana585_2026p006_v001/DST_TRKR_SEED`.
Override `TPCQA_PP`, `TPCQA_OO` or `TPCQA_JOBS` when needed.
Dependencies are checked before reading QA. Logs report per-run availability
and elapsed time; measure the PHGarfield pilot runtime before scaling up.

After success, transfer `qa-direct-pilot.tgz` to the laptop. Extract it beside
`TPC_rDCA_AllRuns_Robust.ipynb`, set `DATA_DIR=Path('qa-direct-pilot')`, and Run All.
The notebook needs NumPy, SciPy, pandas, Matplotlib and IPython, without ROOT.
Check timing availability, the full time spectra, edge zooms and fit quality.
The notebook is self-contained.

## All available runs, same code path

```bash
bash "$TPCQA_WORK/TPC_QA_Direct_QuickFit/run_sdcc.sh" all
```

Transfer/extract `qa-direct-all.tgz`, set `DATA_DIR=Path('qa-direct-all')`, and
Run All again. The cap stays at 100 segments/run in ascending segment order.
Both numeric and historical run-range directories are supported. The wrapper
creates the archive only after the export succeeds.

## Exactly what is exported

- Four rDCA(phi,pT) TH3 families and four charge-count(phi,eta,pT) TH3 families.
  Only the configured pT projections are retained; unused eta-rDCA maps are removed.
- Two small timing spectra, prioritizing native cleaned QA, then standalone
  PHGarfield timing, then production TpcRawHitQA thresholded sample counts.
  Raw sectors 0–11 are North; 12–23 are South. Missing/partial timing is flagged.
- `charge_counts.csv` and `charge_balance.csv`: counts, N+/N-, asymmetry and
  approximate independent-charge errors at each side/pT window.
- `conditions_snapshot.csv`: current-CDB South/North base kEff and R1 GEM-current
  mean/RMS/min/max, valid sample counts, selected channels, BCO limits, global tag,
  payload URLs and query time. Only compact run summaries are exported.
  Current = median across the listed R1 channels at each BCO, then unweighted
  mean over the full conditions payload. A missing channel invalidates that
  side's BCO sample; incomplete coverage is visible. The wrapper explicitly enables the PHGarfield-default Igem recipe when the CDB
  kEff payload is absent. `keff_cdb` stays empty; `keff_from_igem` and
  `keff_reference` contain the derived comparison, with `keff_kind` identifying
  its source. Present invalid coefficients never trigger fallback. Recipe:
  1.55 * (0.185 + 0.815 * Igem / Iref), with Iref=3.36138 uA South and
  3.20357 uA North. Constants and the upstream header blob SHA are saved.
- `phgarfield_reference.csv`: actual PHGarfield integration with full-run-average
  P/T, equal FEE weighting and CDB drift corrections disabled, using CDB geometry.
  One fresh process per run avoids cached CDB URLs leaking across runs.
- `inputs.json`, per-run JSON/NPZ and `manifest.json` retain exact selected files,
  segments, sizes, modification times, settings and checksums. SUCCESS indicates
  no export failures. Incomplete source quantities are explicitly flagged.

`keff_applied` remains empty. Current CDB snapshots and PHGarfield baselines are
not verified historical reconstruction inputs, and full-run currents do not
claim coverage of just the selected 100 segments. No new dE/dx cut is applied.

With `--resume`, unchanged QA projections may be reused after input/settings/code
and checksum verification. CDB snapshots and the requested PHGarfield integration
are freshly queried on every invocation. Original old calibration-QA batch caches
are not inputs. For advanced selection, `export_tpcqa.py --help` exposes the same
pipeline without the wrapper.

## Notebook outputs and definitions

The rDCA working deviance/ndf threshold is 1000. Every slice retains fit quality;
values >10 are flagged as model mismatch even when accepted by that threshold.
Huber c+a sin(phi)+b cos(phi) fits summarize charge-paired means. Primary odd is
(mu+ - mu-)/2; full difference is twice this. Units are explicit in CSV columns.
Reference differences use common accepted phi bins, requiring at least eight.
The robust fit limits isolated angular outliers, not coherent slice-model bias.
Phi-bin bootstrap sensitivity and approximate fit errors are not segment-level
calibration uncertainties. No kEff correction is applied.

Charge-balance plots show all pT windows and both sides. The rDCA-versus-phi map
has no added eta cut; charge counts integrate the producer's stored normal
phi/eta bins (this production uses eta [-1.2,1.2]). No acceptance correction is
inferred from raw charge ratios.

Timing uses median-smoothed, sideband-defined 10/50/90% crossings. Midpoints
supply t0 and CM times; the 10–90% width is not a measurement error. Clock, raw
sample offset and all edge windows are explicit notebook settings. The default
raw shift is 40 bins; verify it using the pilot, not the production name.
No resolved transition means no measured velocity. Data velocity is
(pad-to-CM distance)/((CM-t0)*clock). PHGarfield geometry supplies that distance;
the configured 102.325 cm fallback is labeled when no reference is available.
To use the same distance convention, the notebook converts the integrator's
reported z_pad/time velocity by (z_pad-cm_halfwidth)/z_pad. PHGarfield t0 is its
configured setting, not an independent t0 prediction.

Results in `rdca_odd_all_runs/`: `rdca_odd_robust_per_run.csv` (joined current-CDB
snapshot), `slice_fit_quality.csv`, `charge_balance.csv`, `timing_per_run.csv`,
conditions/reference CSV copies, and complete analysis settings. Figures are
shown inline; automatic per-slice PDFs are absent.

## Clean module

The companion TpcCalibrationQA_clean.zip supplies the minimal native reference
library and these diagnostics in a TrackingAnalysis/TpcCalibrationQA tree.
It removes the old C++ fit/collection implementation, old macros, previous
pilot exporter/notebook, generated outputs and unused eta-rDCA maps. Use its
isolated build-and-pilot script and full SDCC clang-tidy checks before a PR.

## Verification and PR preparation

```bash
python3 -m unittest discover -s TPC_QA_Direct_QuickFit -p 'test_*.py' -v
python3 TPC_QA_Direct_QuickFit/check_notebook.py
```

Local checks cover numerical fits, timing transitions and physical side mapping,
charge arithmetic, directory layouts, caps, summation, resume, CDB process
isolation, current/BCO completeness and the one-command export path. ROOT/CDB I/O
uses a test double locally; the notebook is executed on synthetic two-system
arrays with timing and reference tables. Actual CDB access and the compiled
PHGarfield integration still require the SDCC pilot. Save its log and notebook
outputs before opening a PR. This package does not modify production QA to
record applied corrections; that is a separate reconstruction-provenance change.
