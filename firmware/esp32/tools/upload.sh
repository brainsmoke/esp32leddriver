#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen

MODEL="greatcircles"

if [ "x$1" != "x" ];
then
    MODEL="$1"
fi

echo "$MODEL"
exit 1

for dir in {ani,conf,secret,models}/ models/"$MODEL";
do
	[ -f "$dir" ] || do_echo ampy -p "$device" mkdir "${dir%/}"
done

for f in *.py {ani,conf}/* secret/httpd.json models/"$MODEL"/*;
do
	[ -f "$f" ] && 	do_echo ampy -p "$device" put "$f" "/$f";
done

micropython_reset


