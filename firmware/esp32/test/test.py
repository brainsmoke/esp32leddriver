
import machine, network, utime, gc

import config, model, uartpixel

from ani.testpattern import TestPatternA, TestPatternB

config.load()

leds = model.load()

driver = uartpixel.UartPixel(baudrate = config.uart_baudrate,
                             rx       = config.uart_rx,
                             tx       = config.uart_tx,
                             n        = leds.n_leds)

fb = bytearray(leds.n_leds * 3)
cur_ani = TestPatternB(leds)

try:

    t_next = utime.ticks_add(utime.ticks_us(), 16666)
    while True:
        cur_ani.next_frame(fb)
        dt = utime.ticks_diff(utime.ticks_us(),t_next)
        if dt > 0:
            print("slow")
            print(dt)
            t_next = utime.ticks_add(utime.ticks_us(), 16666)
        else:
            while utime.ticks_diff(utime.ticks_us(),t_next) < 0:
                pass
            t_next = utime.ticks_add(t_next, 16666)
        driver.writefrom(fb)
finally:
    for i in range(len(fb)):
        fb[i] = 0
    driver.writefrom(fb)

