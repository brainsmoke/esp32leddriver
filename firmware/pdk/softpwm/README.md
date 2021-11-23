
# 3-channel 16 bit software PWM LED driver

* 1800Hz, 16-bit PWM (12-bit + 4 bits dithering)
* 3-channels, almost evenly spaced phase offsets
* inverse polarity compared to WS2811 (pin high = LED on)
* UART based signalling (38400 baud, 16 bit values, little endian)
* end-of-frame is determined using a reset delay (between 1-2msec)
* no output line, led index is hard-coded in the firmware
* tested on PFS154 & PMS150C

