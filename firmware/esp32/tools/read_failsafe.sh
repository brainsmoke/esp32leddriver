#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen

magick display <(inkscape -o - --export-type=png --export-dpi=300 -b '#ffffff' \
    <(python3 "${TOOLS_DIR}"/gen_failsave.py -svg-from-json \
        <(ampy -p "$device" get "/secret/failsafe.json")
    )
)
