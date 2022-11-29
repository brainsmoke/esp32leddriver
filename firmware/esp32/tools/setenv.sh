#!/bin/bash

. "$(dirname "$0")/config.sh"

BUILDDIR="$HOME"

ESPIDFDIR="$BUILDDIR/esp-idf"
LEDDRIVERDIR="$BUILDDIR/esp32leddriver"
MPYDIR="$BUILDDIR/micropython"
MPYBUILDDIR="$MPYDIR/ports/esp32"

USER_C_MODULES="\"$LEDDRIVERDIR/firmware/esp32/modules/micropython.cmake\""

BOARD="LEDBALL"
PORT="$DEVICE"

source "$ESPIDFDIR"/export.sh
