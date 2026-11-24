#!/usr/bin/env bash
set -euo pipefail
src=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
export TPCQA_WORK=${TPCQA_WORK:-/sphenix/user/mitrankov/tpcqa-migration-20261123}
export TPCQA_INSTALL=${TPCQA_CLEAN_INSTALL:-$TPCQA_WORK/clean-install}
export TPCQA_BUILD=${TPCQA_BUILD:-$TPCQA_WORK/clean-build}
: "${OFFLINE_MAIN:?Source the same sPHENIX environment used for production QA}"
: "${TPCQA_CDB_TAG:?Set the current-comparison global tag}"
: "${TPCQA_REFERENCE_FIELD:?Set the PHGarfield model reference field in V/cm}"
mkdir -p "$TPCQA_BUILD"
cd "$TPCQA_BUILD"
sh "$src/autogen.sh" --prefix="$TPCQA_INSTALL"
make -j2
make check
export LD_LIBRARY_PATH="$TPCQA_INSTALL/lib:$TPCQA_INSTALL/lib64:${LD_LIBRARY_PATH:-}"
export ROOT_INCLUDE_PATH="$TPCQA_INSTALL/include:${ROOT_INCLUDE_PATH:-}"
# Use a new output root so the compacted reader and library get a fresh pilot.
export TPCQA_WORK="$TPCQA_WORK/clean-validation"
mkdir -p "$TPCQA_WORK"
bash "$src/diagnostics/run_sdcc.sh" pilot
printf '%s\n' "Pilot archive: $TPCQA_WORK/qa-direct-pilot.tgz"
printf '%s\n' "Next: bash $src/tools/validate_sdcc.sh $TPCQA_BUILD"
