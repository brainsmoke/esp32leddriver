#!/bin/bash

DIR="."

if [ "x$1" != "x" ]; then
	DIR="$1"
fi

device="/dev/ttyUSB0"

SHATREE="$(dirname "$0")/../test/shatree.py"
diff <(python "$SHATREE" "$DIR"|sort) <(ampy -p "$device" run "$SHATREE"|sort|tr -d $'\r')|grep '^[<>]'|sed 's:^> [^ ]* :BOARD:'|sed 's:< [^ ]* :HOST:'|sort -t ' ' -k 2

