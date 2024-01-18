#!/bin/bash

. "$(dirname "$0")/config.sh"

check_config

kill_screen

try_echo pushd "$FS_DIR"/

for dir in {ani,conf,secret,webroot,webroot/css,models}/ "$MODEL";
do
	[ -f "$dir" ] || try_echo ampy -p "$device" mkdir --exists-okay "${dir%/}"
done

for f in *.py ani/* secret/httpd.json webroot/css/*;
do
	[ -f "$f" ] && try_echo ampy -p "$device" put "$f" "/$f";
done

try_echo popd

try_echo pushd "$MODEL_DIR"/

for f in *;
do
	[ -f "$f" ] && try_echo ampy -p "$device" put "$f" "$MODEL/$f";
done

try_echo popd

try_echo pushd "$CONFIG_DIR"/

for f in *;
do
	[ -f "$f" ] && try_echo ampy -p "$device" put "$f" "/conf/$f";
done

try_echo popd

micropython_reset

