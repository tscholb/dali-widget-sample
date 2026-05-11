#!/bin/bash
#
# Build all DALi widget sample packages sequentially via gbs.
#
# Usage:
#   ./build-all.sh                 # default: armv7l
#   ./build-all.sh -A aarch64      # override arch / pass extra gbs args
#   ./build-all.sh --clean-once
#
set -e

SAMPLES=(
    widget-app-template
    widget-viewer-template
    widget-app-interactive
    widget-viewer-interactive
    widget-app-component
    widget-viewer-component
)

GBS_ARGS=("$@")
if [[ ! " $* " =~ " -A " ]]; then
    GBS_ARGS=(-A armv7l "${GBS_ARGS[@]}")
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

for sample in "${SAMPLES[@]}"; do
    echo ""
    echo "================================================================"
    echo " Building: $sample"
    echo "================================================================"
    gbs build --include-all --packaging-dir "$sample/packaging" "${GBS_ARGS[@]}"
done

echo ""
echo "All samples built successfully."
