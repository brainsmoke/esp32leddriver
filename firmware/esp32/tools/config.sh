#!/bin/bash

die() {
	echo $'\033[1;31m'"$@"$'\033[0m' >&2
	exit 1
}

do_echo() {
	echo $'\033[0;32m'"$@"$'\033[0m' >&2
	$@
}

message() {
	echo $'\033[1;37m'"$@"$'\033[0m' >&2
}

try_echo() {
	do_echo "$@" || die "error: $?"
}

check_model() {
	message "Using model: $MODEL"
	[ -d "$(dirname "$0")/../conf/$MODEL" ] || \
	die "No config found for model: \"$(dirname "$0")/../conf/$MODEL\" does not exist"
}


kill_screen() {
	message "Killing any attached micropython screens"
	screen -X -S micropython quit
	sleep .4
}

micropython_reset() {
	message "Resetting device"
	echo $'\x04' > "$device"
}

if [ "x$ESPTTY" != "x" ]; then
	if [ ! -c "ESPTTY" ]; then
		die "Bad tty: \$ESPTTY \"$ESPTTY\""
	fi
	device="$ESPTTY"

elif [ -c /dev/ttyACM0 ]; then
	device=/dev/ttyACM0
elif [ -c /dev/ttyUSB0 ]; then
	device=/dev/ttyUSB0
else
	echo "no serial found"
	[ -n "$IGNORE_SERIAL" ] || exit 1
fi

MODEL="greatcircles"

if [ "x$1" != "x" ];
then
    MODEL="$1"
fi


