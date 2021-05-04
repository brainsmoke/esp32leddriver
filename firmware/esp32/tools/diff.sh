
device=/dev/ttyUSB0

screen -X -S micropython quit
sleep .4

for f in "$@"; do 
    diff -u <(ampy -p "$device" get "$f"|tr -d $'\r'|head -n -1) "$f"
done

