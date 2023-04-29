#!/bin/bash
#
# Usage: ./change_address.sh <led-address>
#

. "$( dirname "$0" )"/config.sh

ADDRESS="$1"

run easypdkprog -p "$PROG" -n "$PART" read "$TMP_IN" && \
run python3 test/settings.py "$ARCH" "$TMP_IN" change_address "$ADDRESS" > "$TMP_OUT" && \
run easypdkprog -p "$PROG" -n "$PART" --nocalibrate --noblankchk --noerase write "$TMP_OUT" && \
run easypdkprog -p "$PROG" -n "$PART" read "$TMP_IN" && \
(
	ADDRESS_READ="$(run python3 test/settings.py "$ARCH" "$TMP_IN" get_address)"

	if [ "x$ADDRESS" = "x$ADDRESS_READ" ]; then
		echo "Success: $ADDRESS_READ"
	else
		echo "Failure"
	fi
)

