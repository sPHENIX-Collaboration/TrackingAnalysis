#!/usr/bin/env bash
set -euo pipefail
src=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
build=${1:?Usage: bash tools/validate_sdcc.sh /absolute/build/directory}
: "${OFFLINE_MAIN:?Source the sPHENIX environment}"
: "${TPCQA_INSTALL:?Set the installation prefix for the clean module}"
for tool in clang-tidy clang-format root-config; do
  command -v "$tool" >/dev/null || { echo "Missing required check: $tool" >&2; exit 2; }
done
clang-format --dry-run --Werror "$src"/*.cc "$src"/*.h "$src"/tests/*.cc
python3 "$src/tools/compile_database.py" "$build"
for file in "$src"/*.cc "$src"/tests/*.cc; do
  clang-tidy --warnings-as-errors='*' -p "$build" "$file"
done
make -C "$build" check
printf '%s\n' 'Full native clang-format, clang-tidy and C++ checks passed.'
