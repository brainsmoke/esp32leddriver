
# m02812: WS2812 bitbang routine for a 48MHz cortex m0


```
make
gdb-multiarch dither16.elf
set mem inaccessible-by-default off
target extended-remote /dev/ttyACM0
monitor swdp_scan
attach 1
load
run
```

# Template code

Based on the template code at:
https://github.com/asquared/stm32f0-barebones-template

which is a stripped-down version of:
https://github.com/szczys/stm32f0-discovery-basic-template

