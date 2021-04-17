
import machine, network, utime, gc

import config, model, uartpixel, cball

from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble

import esp
esp.osdebug(None)

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

if config.essid != None:
    wifi_connect(config.essid, config.password)

leds = model.load()

driver = uartpixel.UartPixel(baudrate = config.uart_baudrate,
                             rx       = config.uart_rx,
                             tx       = config.uart_tx,
                             n        = leds.n_leds)

from esphttpd import HTTP_Server, redirect
server = HTTP_Server()

ani = [
    Orbit(leds),
    Lorenz(leds),
    Fire(leds),
    Gradient(leds),
    Spiral(leds),
    Wobble(leds),
]

class Off:
    def next_frame(self, fbuf):
        cball.bytearray_memset(fbuf, 0)

off = Off()


index_html = """<body>

<form action="/previous" method="POST"><input type="submit" value="&lt;&lt;"></form>
<form action="/next" method="POST"><input type="submit" value="&gt;&gt;"></form>
<form action="/on" method="POST"><input type="submit" value="On"></form>
<form action="/off" method="POST"><input type="submit" value="Off"></form>
"""

def animate():

    cur = 0
    cur_ani = ani[cur]

    fadeout = bytearray(leds.n_leds * 3)
    fb = bytearray(leds.n_leds * 3)

    fade = 60

    def set_animation(next_ani):
        nonlocal cur_ani, fade, fb, fadeout
        cur_ani = next_ani
        fade = 0
        fb, fadeout = fadeout, fb

    try:
        @server.route("/")
        def index(req):
            req.write(index_html)
            req.write("<h1>{}\n".format(str(cur_ani.__qualname__)))

        @server.route("/next", "POST")
        def next(req):
            nonlocal cur
            print("[next]")
            if cur_ani != off:
                cur = (cur+1)%len(ani)
                set_animation(ani[cur])
            redirect(req, "/")

        @server.route("/previous", "POST")
        def previous(req):
            nonlocal cur
            print("[previous]")
            if cur_ani != off:
                cur = (cur+1)%len(ani)
                set_animation(ani[cur])
            redirect(req, "/")

        @server.route("/off", "POST")
        def off_(req):
            print("[off]")
            set_animation(off)
            redirect(req, "/")

        @server.route("/on", "POST")
        def on_(req):
            print("[on]")
            set_animation(ani[cur])
            redirect(req, "/")

        server.start()

        t_next = utime.ticks_add(utime.ticks_us(), 16666)
        while True:
            cur_fb = fb
            cur_fade = fadeout
            cur_ani.next_frame(cur_fb)
            if fade < 60:
                fade += 1
                cball.bytearray_blend(cur_fb, cur_fade, cur_fb, fade/60.)

            dt = utime.ticks_diff(t_next, utime.ticks_us())
            if dt < -2000:
                print(dt)
                t_next = utime.ticks_us()

            elif dt > 0:
                while utime.ticks_diff(utime.ticks_us(),t_next) < 0:
                    pass
#                utime.sleep_us(dt)

            t_next = utime.ticks_add(t_next, 16666)
            driver.writefrom(cur_fb)
    finally:
        for i in range(len(fb)):
            fb[i] = 0
        driver.writefrom(fb)
        server.stop()
        server.del_route("/next")
        server.del_route("/previous")

animate()

