
. "$(dirname "$0")/config.sh"

kill_screen

for f in "$@"; do 
    diff -u <(ampy -p "$device" get "$f"|tr -d $'\r'|head -n -1) "$f"
done

micropython_reset
