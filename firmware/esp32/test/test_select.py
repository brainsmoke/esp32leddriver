
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

    c = []
    cur = []
    done = []
    while True:
        for i in range(0,240,4):
            for a,b in ( (i, i+2), (i+1, i+3), ):

                if a in done:
                    continue

                for i in range(0,240*3):
                    fb [ i ] = 0
                for i in range(0,240):
                    if i in done:
                        fb [ i*3 + 2 ] = 32
                for i in range(0,240):
                    if i in cur:
                        fb [ i*3 + 1 ] = 63
                        fb [ (i+2)*3 + 1 ] = 63



                fb [ a*3  ] = 127
                fb [ a*3+1] = 127
                fb [ b*3  ] = 127
                fb [ b*3+1] = 127
                driver.writefrom(fb)
                print (a)
                x=input()

                if x == 'a':
                    cur += [a]
                    done += [a,b]
                    print (cur)
                if x == 'x':
                    c.append(cur)
                    cur = []
                    print (c)
            
finally:
    for i in range(len(fb)):
        fb[i] = 0
    driver.writefrom(fb)

