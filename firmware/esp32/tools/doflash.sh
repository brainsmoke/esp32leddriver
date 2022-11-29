#!/bin/bash

. "$(dirname "$0")/setenv.sh"

kill_screen

make -C "$MPYBUILDDIR" "USER_C_MODULES=$USER_C_MODULES" "BOARD=$BOARD" "PORT=$PORT" deploy
