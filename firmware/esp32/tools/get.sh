. "$(dirname "$0")/config.sh

kill_screen

for f in "$@"; do 
    do_echo ampy -p "$device" get "$f";
done

micropython_reset

