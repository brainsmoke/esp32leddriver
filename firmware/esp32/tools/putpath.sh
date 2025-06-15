#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen
try_echo ampy -p "$device" put "$1" "$2/$(basename "$1")";

