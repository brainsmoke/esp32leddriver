
import machine, utime, gc, re

import config, model, uartpixel, cball, configform, csrf
from conf.load import get_animations

from ani.gradient import ConfigMode

import esp
#esp.osdebug(None)
import _thread
_thread.stack_size(8192)

import machine
machine.freq(240000000)

config.load()

leds = model.load()

driver = uartpixel.UartPixel(baudrate = config.uart_baudrate,
                             rx       = config.uart_rx,
                             tx       = config.uart_tx,
                             n        = leds.n_leds,
                             led_order= 'RGB',
                             brightness = config.brightness,
                             cutoff   = 0,
                             remap    = None,
                             fps      = config.fps,
                             framebuf = False )

fb = driver.create_framebuffer16()

for i in range(len(fb)):
    fb[i-1] = 0x1000
    fb[i] = 0xffff
    driver.writefrom16(fb)
    input()


