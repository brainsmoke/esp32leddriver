
device=/dev/ttyUSB0

do_echo() {
	echo "> $@"
	$@
}

screen -X -S micropython quit
sleep .4

for f in "$@"; do 
    do_echo ampy -p "$device" get "$f";
done

