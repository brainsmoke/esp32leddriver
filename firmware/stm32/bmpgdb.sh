#!/bin/bash

gdb-multiarch -x "$(dirname $0)/bmp.gdb" "$1"

