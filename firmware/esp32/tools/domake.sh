#!/bin/bash

IGNORE_SERIAL=1

. "$(dirname "$0")/setenv.sh"

make -C "$MPYBUILDDIR" "USER_C_MODULES=$USER_C_MODULES" "BOARD=$BOARD" all
