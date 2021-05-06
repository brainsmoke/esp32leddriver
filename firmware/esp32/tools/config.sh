
device=/dev/ttyUSB0

do_echo() {
	echo "> $@"
	$@
}

kill_screen() {
	screen -X -S micropython quit
	sleep .4
}

micropython_reset() {
	echo $'\x04' > "$device"
}
