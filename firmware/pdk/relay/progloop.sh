#!/bin/bash

while read a; do
	~/easy-pdk-programmer-software/easypdkprog -p /dev/ttyACM0 -n PMS150C -v write relay_newpcb-pdk13.ihx && \
	~/easy-pdk-programmer-software/easypdkprog -p /dev/ttyACM0 -n PMS150C read readoutx.hex && \
	echo "only diff should (may) be the clock calibration value" && \
	diff readout-pdk13.hex readoutx.hex
done
