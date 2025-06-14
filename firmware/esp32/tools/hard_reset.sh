#!/bin/bash

. "$(dirname "$0")/config.sh"

${ESPTOOL} --chip esp32 -p "$device" -b 115200 --before=default_reset --after=hard_reset run

