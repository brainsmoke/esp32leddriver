#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen

do_echo ampy -p "$device" put <(python3 "$(dirname "$0")/gen_failsave.py" -json "$1") "/secret/failsafe.json";
do_echo ampy -p "$device" get "/secret/failsafe.json";

micropython_reset

