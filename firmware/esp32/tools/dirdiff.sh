#!/bin/bash

. "$(dirname "$0")/config.sh"

kill_screen
check_config

SHATREE="${BASE_DIR}/test/shatree.py"

diff <((

	python3 "$SHATREE" "$FS_DIR" ""
	python3 "$SHATREE" "$CONFIG_DIR" "/conf"
	python3 "$SHATREE" "$MODEL_DIR" "$MODEL"

)|sort) <((

	ampy -p "$device" run "$SHATREE"

)|sort|tr -d $'\r')|grep '^[<>]'|sed 's:^> [^ ]* :BOARD:'|sed 's:< [^ ]* :HOST:'|sort -t ' ' -k 2

micropython_reset
