#!/bin/bash

gdb-multiarch -x "$(dirname $0)/bmpflash.gdb" "$1"

