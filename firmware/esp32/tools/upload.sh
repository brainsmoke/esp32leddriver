
device=/dev/ttyUSB0

do_echo() {
	echo "> $@"
	$@
}

screen -X -S micropython quit
sleep .4

for dir in {ani,conf,secret,models}/ models/*;
do
	[ -f "$dir" ] || do_echo ampy -p "$device" mkdir "${dir%/}"
done

for f in *.py {ani,conf,secret}/* models/*/*;
do
	[ -f "$f" ] && 	do_echo ampy -p "$device" put "$f" "/$f";
done

echo $'\x04' > "$device"


