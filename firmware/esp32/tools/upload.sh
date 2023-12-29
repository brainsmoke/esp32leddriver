#!/bin/bash

. "$(dirname "$0")/config.sh"

check_model

kill_screen

try_echo pushd "$(dirname "$0")"/../fs/

for dir in {ani,conf,secret,models}/ models/"$MODEL";
do
	[ -f "$dir" ] || try_echo ampy -p "$device" mkdir --exists-okay "${dir%/}"
done

for f in *.py ani/* secret/httpd.json models/"$MODEL"/*;
do
	[ -f "$f" ] && try_echo ampy -p "$device" put "$f" "/$f";
done

try_echo popd

try_echo pushd "$(dirname "$0")"/../conf/"$MODEL"/

for f in *;
do
	[ -f "$f" ] && try_echo ampy -p "$device" put "$f" "/conf/$f";
done

try_echo popd

micropython_reset

