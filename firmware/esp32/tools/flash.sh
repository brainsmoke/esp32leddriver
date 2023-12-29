#!/bin/bash
. "$(dirname "$0")/config.sh"

message "Checking for tools"
try_echo which esptool.py
try_echo which ampy

check_model

kill_screen

try_echo esptool.py --port "$device" erase_flash
try_echo esptool.py --chip esp32 -p "$device" -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 16MB 0x1000 "$(dirname "$0")/../bin/firmware-ledball.bin"
try_echo sleep 3
try_echo ../tools/upload.sh "$1"

