#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen

for dir in {ani,conf,secret,models}/ models/*;
do
	[ -f "$dir" ] || do_echo ampy -p "$device" mkdir "${dir%/}"
done

for f in *.py {ani,conf}/* secret/httpd.json models/*/*;
do
	[ -f "$f" ] && 	do_echo ampy -p "$device" put "$f" "/$f";
done

micropython_reset


