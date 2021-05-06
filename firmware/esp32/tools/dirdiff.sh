#!/bin/bash

. "$(dirname "$0")/config.sh

SHATREE="$(dirname "$0")/../test/shatree.py"

DIR="."
if [ "x$1" != "x" ]; then
	DIR="$1"
fi

diff <(python "$SHATREE" "$DIR"|sort) <(ampy -p "$device" run "$SHATREE"|sort|tr -d $'\r')|grep '^[<>]'|sed 's:^> [^ ]* :BOARD:'|sed 's:< [^ ]* :HOST:'|sort -t ' ' -k 2

micropython_reset
