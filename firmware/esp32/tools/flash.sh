#!/bin/bash
. "$(dirname "$0")/config.sh"

kill_screen

do_echo esptool.py --port "$device" erase_flash
do_echo esptool.py --chip esp32 -p "$device" -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 16MB 0x1000 "$(dirname "$0")/../bin/firmware-ledball.bin"
cd "$(dirname "$0")/../fs"
do_echo sleep 2
do_echo ../tools/upload.sh

micropython_reset

