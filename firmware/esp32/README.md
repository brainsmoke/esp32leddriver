# ESP32 firmware upload / build

## Upload pre-built firmware

```bash
# make sure you are a member of the dailout group (so you can talk to the serial device)
sudo usermod -a -G dialout "$USER"
# you might need to log in again for it to take effect or you could do:
# (but newgrp only works for the current shell)
newgrp -

# install pip (using whatever package manager you have)
sudo apt install python3-pip

sudo pip3 install esptool
sudo pip3 install adafruit-ampy

# the following assumes the serial device belonging to the 
esptool.py --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 -p /dev/ttyUSB0 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 16MB 0x1000 bin/firmware-ledball.bin
cd fs
../tools/upload.sh
screen /dev/ttyUSB0 115200
ctrl-c
>>>import setup
(enter wifi creds, will be stored in flash)
ctrl-d

```

## Build firmware

```bash

# should be correct, but not tested from a clean VM

git clone https://github.com/brainsmoke/esp32leddriver

# ESP-IDF
git clone -b v4.2 --recursive https://github.com/espressif/esp-idf.git

# clone of micropython using esp32 native vfs, also has settings for http[s] server /mbedtls
git clone https://github.com/brainsmoke/micropython -b leddriver
cd micropython/port/esp32

# Edit setenv.sh to set the correct build directories:
"$EDITOR" setenv.sh

./domake.sh
# firmware should be in build-LEDBALL/firmware.bin

./doflash.sh


```

