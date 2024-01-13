#!/bin/bash

TOOLS_DIR="$(dirname "$BASH_SOURCE" )"
BASE_DIR="$(dirname "$TOOLS_DIR" )"

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

get_model() {
    "${TOOLS_DIR}/get_model.py" "${BASE_DIR}/conf/${CONFIG}" ||\
	die "cannot determine model"
}

check_config() {
	message "Using configuration: ${CONFIG}"

	CONFIG_DIR="${BASE_DIR}/conf/${CONFIG}"
	[ -d "${CONFIG_DIR}" ] || \
	die "Configuration not found: \"${CONFIG_DIR}\" does not exist"

	FS_DIR="${BASE_DIR}/fs"
	[ -d "${FS_DIR}" ] || \
	die "Filesystem directory not found: \"${FS_DIR}\" does not exist"

	MODEL="$(get_model)"
	message "Using model: ${MODEL}"

	MODEL_DIR="${BASE_DIR}/${MODEL}"
	[ -d "${MODEL_DIR}" ] || \
	die "Model directory not found: \"${MODEL_DIR}\" does not exist"
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
	if [ ! -c "$ESPTTY" ]; then
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

CONFIG="greatcircles"

if [ "x$1" != "x" ];
then
    CONFIG="$1"
fi


