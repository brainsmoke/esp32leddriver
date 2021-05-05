# ESP32 firmware upload / build

## Upload pre-built firmware

```bash
pip3 install esptool
esptool.py --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 -p /dev/ttyUSB0 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 16MB 0x1000 bin/firmware-ledball.bin
cd fs
../tools/upload.sh
screen /dev/ttyUSB0 115200
ctrl-d
>>>import setup
(enter wifi creds, will be stored in flash)
ctrl-d

```

## Build firmware

```bash

# just mental notes now, need to test!

git clone https://github.com/brainsmoke/esp32leddriver

# ESP-IDF
git clone -b v4.2 --recursive https://github.com/espressif/esp-idf.git

# clone of micropython using esp32 native vfs, also has settings for http[s] server /mbedtls
git clone https://github.com/brainsmoke/micropython -b leddriver
cd micropython/posrt/esp32

# XXX: Edit do{make,flash}.sh files with correct directories

./domake.sh
# firmware should be in build-LEDBALL/firmware.bin

./doflash.sh


```

