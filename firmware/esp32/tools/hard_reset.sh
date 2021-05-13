#!/bin/bash

. "$(dirname "$0")/config.sh"

esptool.py --chip esp32 -p "$device" -b 460800 --before=default_reset --after=hard_reset run

