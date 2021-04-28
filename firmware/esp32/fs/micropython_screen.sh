#!/bin/sh

device=/dev/ttyUSB0

screen -d -m -S micropython "$device" 115200

screen -r -S micropython
