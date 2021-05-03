
import machine, network, utime, gc

import config, model, uartpixel

config.load()

leds = model.load()

driver = uartpixel.UartPixel(baudrate = config.uart_baudrate,
                             rx       = config.uart_rx,
                             tx       = config.uart_tx,
                             n        = leds.n_leds,
                             remap    = leds.remap)

fb = bytearray(leds.n_leds * 3)

try:

    while True:
        for i in range(0,240,4):
            for a,b in ( (i, i+2), (i+1, i+3), ):

                for i in range(0,240*3):
                    fb [ i ] = 0

                fb [ a*3  ] = 127
                fb [ a*3+1] = 32
                fb [ b*3  ] = 32
                fb [ b*3+1] = 12
                driver.writefrom(fb)
                print (a)
                x=input()
            
finally:
    for i in range(len(fb)):
        fb[i] = 0
    driver.writefrom(fb)

