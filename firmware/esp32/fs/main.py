
import machine, network, utime, gc

import config, model, uartpixel

#from ani.orbit import Orbit
from ani.lorenz import Lorenz
#from ani.fire import Fire
#from ani.gradient import Gradient, Spiral, Wobble

def wifi_connect(essid, password):

    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if wlan.isconnected():
        wlan.disconnect()

    print('connecting to network...')
    wlan.connect(essid, password)
    while not wlan.isconnected():
        pass
    print(wlan.ifconfig())

def create_listener_socket(port):
    addr = usocket.getaddrinfo('0.0.0.0', port)[0][-1]
    s = usocket.socket()
    s.bind(addr)
    s.listen(1)
    return s

def get_connection(socket):
    conn, addr = socket.accept()
    return conn.makefile('rb')

config.load()

#if config.essid != None:
#    wifi_connect(config.essid, config.password)

leds = model.load()

driver = uartpixel.UartPixel(baudrate = config.uart_baudrate,
                             rx       = config.uart_rx,
                             tx       = config.uart_tx,
                             n        = leds.n_leds)

fb = bytearray(leds.n_leds * 3)
#ani0 = Orbit(leds)
ani1 = Lorenz(leds)
#ani2 = Fire(leds)
#ani3 = Gradient(leds)
#ani4 = Spiral(leds)
#ani5 = Wobble(leds)
cur_ani = ani1

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

