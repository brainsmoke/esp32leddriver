
if [ -e /dev/ttyACM0 ]; then
device=/dev/ttyACM0
elif [ -e /dev/ttyUSB0 ]; then
device=/dev/ttyUSB0
else
echo "no serial found"
[ -n "$IGNORE_SERIAL" ] || exit 1
fi

do_echo() {
	echo "> $@"
	$@ || (echo "error: $?"; exit)
}

kill_screen() {
	screen -X -S micropython quit
	sleep .4
}

micropython_reset() {
	echo $'\x04' > "$device"
}
