#!/bin/bash

. "$(dirname "$0")/setenv.sh"

kill_screen

make -C "$MPYBUILDDIR" "EXTRA_IDFPY_FLAGS=-DUSER_MODULES=$USER_MODULES" "BOARD=$BOARD" "PORT=$PORT" deploy
