#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen
try_echo ampy -p "$device" run "$1";
screen -S micropython "$device" 115200
micropython_reset
