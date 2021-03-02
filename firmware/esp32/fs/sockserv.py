
import machine, network, usocket

import config, model, uartpixel

from ani.orbit import Orbit

def wifi_connect(essid, password):

    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if wlan.isconnected():
        wlan.disconnect()

    print('connecting to network...')
    wlan.connect(essid, password)
    while not wlan.isconnected():
        pass
    print(wlan.config('mac'))
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

if config.essid != None:
    wifi_connect(config.essid, config.password)

leds = model.load()

outbuf = bytearray(leds.n_leds * 3 * 2 + 4)
outbuf[-4:] = b"\xff\xff\xff\xf0"
bufview = memoryview(outbuf)[:-4]

uart = machine.UART(1, baudrate = config.uart_baudrate,
                       rx       = config.uart_rx,
                       tx       = config.uart_tx,
                       txbuf    = leds.n_leds * 3 * 2 + 4)

s = create_listener_socket(config.port)

while True:
    conn = get_connection(s)
    while conn.readinto(bufview) == len(bufview):
        uart.write(outbuf)
    conn.close()

