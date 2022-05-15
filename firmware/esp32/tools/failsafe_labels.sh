#!/bin/bash

inkscape --export-filename="$2" <(python3 "$(dirname "$0")/gen_failsave.py" -svg "$1")

