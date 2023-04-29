#!/bin/bash

run() {
	echo $'\033[0;32m'"$@"$'\033[0m' >&2
	"$@"
}

PROG="/dev/ttyACM0"

TMP_IN="_read.hex"
TMP_OUT="_write.hex"

PART="$(run easypdkprog -p "$PROG" probe |grep 'IC is supported'|sed 's/IC is supported: \([^ ]*\).*/\1/')"

ARCH="unknown"

if [ x"$PART" = x"PMS150C" ]; then
	ARCH="pdk13"
	FIRMWARE="leddriver16bit-pms150c.ihx"
fi

if [ x"$PART" = x"PFS154" ]; then
	ARCH="pdk14"
	FIRMWARE="leddriver16bit-pfs154.ihx"
fi

if [ x"$ARCH" = x"unknown" ]; then
	echo "cannot determine part"
	exit 1
fi

