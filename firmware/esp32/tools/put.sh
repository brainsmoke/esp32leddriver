#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen

for f in "$@"; do 
    try_echo ampy -p "$device" put "$f" "/$f";
done

micropython_reset

