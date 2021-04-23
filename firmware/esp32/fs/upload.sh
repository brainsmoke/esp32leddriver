
device=/dev/ttyUSB0

do_echo() {
	echo "> $@"
	$@
}

for dir in {ani,conf,secret,models}/ models/*;
do
	[ -f "$dir" ] || do_echo ampy -p "$device" mkdir "${dir%/}"
done

for f in *.py {ani,conf,secret}/* models/*/*;
do
	[ -f "$f" ] && 	do_echo ampy -p "$device" put "$f" "/$f";
done

