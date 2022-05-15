#!/bin/bash

. "$(dirname "$0")/config.sh"

kill_screen

SHATREE="$(dirname "$0")/../test/shatree.py"

DIR="."
if [ "x$1" != "x" ]; then
	DIR="$1"
fi

diff <(python3 "$SHATREE" "$DIR"|sort) <(ampy -p "$device" run "$SHATREE"|sort|tr -d $'\r')|grep '^[<>]'|sed 's:^> [^ ]* :BOARD:'|sed 's:< [^ ]* :HOST:'|sort -t ' ' -k 2

micropython_reset
