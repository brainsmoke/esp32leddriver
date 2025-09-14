#!/bin/bash
ESPTTY=/dev/null
. "$(dirname "$0")/config.sh"

number="$1"

magick display <(inkscape -o - --export-type=png --export-dpi=300 -b '#ffffff' \
    <(python3 "${TOOLS_DIR}"/gen_failsave.py -svg-from-json \
        <(python3 "${TOOLS_DIR}"/gen_failsave.py -json "$number")
    )
)
