
device=/dev/ttyUSB0

do_echo() {
	echo "> $@"
	$@
}

screen -X -S micropython quit

do_echo ampy -p "$device" run "$1";
screen -S micropython "$device" 115200

