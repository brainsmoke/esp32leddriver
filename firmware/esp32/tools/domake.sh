#!/bin/bash

. "$(dirname "$0")/setenv.sh"

make -C "$MPYBUILDDIR" "EXTRA_IDFPY_FLAGS=-DUSER_MODULES=$USER_MODULES" "BOARD=$BOARD" all
