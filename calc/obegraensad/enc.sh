#!/bin/bash

ffmpeg -i "$1" -c:v rawvideo -f rawvideo -pix_fmt gray8 - |python3 obegenc.py "$2" "$3" "$4"
