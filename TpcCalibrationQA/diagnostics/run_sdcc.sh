#!/usr/bin/env bash
set -euo pipefail
mode=${1:-pilot}
case "$mode" in
  pilot) selection=(--select run3pp=81558 --select run3oo=82688) ;;
  all) selection=(--all-runs) ;;
  *) echo 'Usage: bash run_sdcc.sh pilot|all' >&2; exit 2 ;;
esac
export TPCQA_WORK=${TPCQA_WORK:-/sphenix/user/mitrankov/tpcqa-migration-20261123}
export TPCQA_INSTALL=${TPCQA_INSTALL:-$TPCQA_WORK/install}
: "${TPCQA_CDB_TAG:?Set the CDB global tag for this current-snapshot comparison}"
: "${TPCQA_REFERENCE_FIELD:?Set the PHGarfield reference field in V/cm}"
if [[ -n ${TPCQA_INSTALL:-} ]]; then
  export LD_LIBRARY_PATH="$TPCQA_INSTALL/lib:$TPCQA_INSTALL/lib64:${LD_LIBRARY_PATH:-}"
  export ROOT_INCLUDE_PATH="$TPCQA_INSTALL/include:${ROOT_INCLUDE_PATH:-}"
fi
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
python3 -u "$script_dir/export_tpcqa.py" \
  --source "run3pp=${TPCQA_PP:-/sphenix/data/data02/sphnxpro/production/run3pp/physics/ana585_2026p006_v001/DST_TRKR_SEED}" \
  --source "run3oo=${TPCQA_OO:-/sphenix/data/data02/sphnxpro/production/run3oo/physics/ana585_2026p006_v001/DST_TRKR_SEED}" \
  "${selection[@]}" --max-files 100 --jobs "${TPCQA_JOBS:-2}" \
  --pt-windows 0.5:1,1:2,2:4 \
  --cdb-tag "$TPCQA_CDB_TAG" --reference-field "$TPCQA_REFERENCE_FIELD" --keff-fallback igem \
  --resume --out "$TPCQA_WORK/qa-direct-$mode"
tar -C "$TPCQA_WORK" -czf "$TPCQA_WORK/qa-direct-$mode.tgz" "qa-direct-$mode"
echo "Ready to transfer: $TPCQA_WORK/qa-direct-$mode.tgz"
