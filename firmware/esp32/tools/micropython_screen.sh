#!/bin/bash

. "$(dirname "$0")/config.sh"

screen -S micropython "$device" 115200
