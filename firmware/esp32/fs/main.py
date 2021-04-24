
import machine, network, utime, gc, re

import config, model, uartpixel, cball

from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble

import esp
#esp.osdebug(None)

import machine
machine.freq(240000000)


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

if config.use_tls:
    server = HTTP_Server(True, config.key_file, config.cert_file) # https
else:
    server = HTTP_Server()

import webdir
webdir.add_webdir(server, '/script', '/webroot/script', index=True, compression='gzip')
webdir.add_webdir(server, '/css', '/webroot/css', index=True, compression='gzip')
webdir.add_webdir(server, '/models', '/models', index=True)

ani = [
    Lorenz(leds),
    Fire(leds),
    Gradient(leds),
    Orbit(leds),
    Spiral(leds),
    Wobble(leds),
]

class Off:
    def next_frame(self, fbuf):
        cball.bytearray_memset(fbuf, 0)

off = Off()


def write_index(req, animation, is_on, brightness, gamma):
    req.write("""<head>
<link rel="icon" href="data:,">
<body>

<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style type="text/css">
body
{
	font-family: Sans;
}

main
{
    display: grid;
    grid-template: "a b c" auto
                   "d d d" auto
                   "e e e" auto
                   "f f f" 2em / 2fr 3fr 2fr;
    max-width: 30em;
    margin-left: auto;
    margin-right: auto;
}

form
{
    display: contents;
}

input[type=submit]
{
    height: 2em;
    font-size: 200%
}

h1,h2
{
    text-align: center;
}

h2
{
	font-size: 14pt;
	margin-bottom: .2em;
}

#brightness
{
    grid-area: e;
}

#gamma
{
    grid-area: f;
}

.settings
{
    grid-area: d;
}

input[type=range]
{
    width: 100%;
}
</style>
<body>
<main>
<form>
""")
    if is_on:
        req.write("""

<input type="submit" value="<<" formmethod="POST" formaction="/previous">
<input type="submit" value="Off" formmethod="POST" formaction="/off">
<input type="submit" value=">>" formmethod="POST" formaction="/next">""")
    else:
        req.write("""
<input type="submit" value="<<" formmethod="POST" formaction="/previous" disabled>
<input type="submit" value="On" formmethod="POST" formaction="/on">
<input type="submit" value=">>" formmethod="POST" formaction="/next" disabled>
""")
    req.write("""
</form>
<form id="brightness_form" action="/set/brightness" method="POST">
<div id="brightness"><h2>brightness</h2>
<input name="value" type="range" min=".02" max="1" step="0.02" value="{:f}" onchange="document.getElementById('brightness_form').submit();">
</div>
</form>
<form id="gamma_form" action="/set/gamma" method="POST">
<div id="gamma"><h2>gamma</h2>
<input name="value" type="range" min="1" max="4" step="0.1" value="{:f}" onchange="document.getElementById('gamma_form').submit();">
</div>
</form>
<div class="settings">""".format(brightness, gamma))
    req.write("<h1>{}\n".format(animation))
    req.write("""
</div>
</main>""")

def animate():

    cur = 0
    cur_ani = ani[cur]

    fadeout = bytearray(leds.n_leds * 3)
    fb = bytearray(leds.n_leds * 3)

    fade = 60

    old_gamma = new_gamma = driver.gamma
    old_brightness = new_brightness = driver.brightness
    gamma_fade = 60

    def set_animation(next_ani):
        nonlocal cur_ani, fade, fb, fadeout
        cur_ani = next_ani
        fade = 0
        fb, fadeout = fadeout, fb

    try:
        @server.route("/")
        def index(req):
            write_index(req, str(cur_ani.__qualname__), cur_ani!=off, new_brightness, new_gamma)

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

        # quick hack :-P
        arr = bytearray(256)
        regex = re.compile('(^|&)value=([^&]*)(&|$)')
        def get_val(req):
            n = req.recv(arr)
            g=regex.search(bytes(arr[:n]))
            return float(g.group(2))

        @server.route("/set/brightness", "POST")
        def brightness(req):
            nonlocal old_brightness, new_brightness, old_gamma, gamma_fade
            print("[brightness]")
            value = get_val(req)
            if 0 <= value <= 1:
                old_brightness = driver.get_brightness()
                old_gamma = driver.get_gamma()
                gamma_fade = 0
                new_brightness = value
            redirect(req, "/")

        @server.route("/set/gamma", "POST")
        def brightness(req):
            nonlocal old_brightness, new_gamma, old_gamma, gamma_fade
            print("[gamma]")
            value = get_val(req)
            if 1 <= value <= 10:
                old_brightness = driver.get_brightness()
                old_gamma = driver.get_gamma()
                gamma_fade = 0
                new_gamma = value
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

            if gamma_fade < 60:
                gamma_fade += 1
                cur_gamma = ( new_gamma*gamma_fade + old_gamma*(60-gamma_fade) ) / 60
                cur_brightness = ( new_brightness*gamma_fade + old_brightness*(60-gamma_fade) ) / 60
                driver.calc_gamma_map(gamma=cur_gamma, brightness=cur_brightness)

            dt = utime.ticks_diff(t_next, utime.ticks_us())
            if dt < -2000:
                print(dt)
                t_next = utime.ticks_us()

            elif dt > 0:
#                while utime.ticks_diff(utime.ticks_us(),t_next) < 0:
#                    pass
                utime.sleep_us(dt)

            t_next = utime.ticks_add(t_next, 16666)
            driver.writefrom(cur_fb)
    finally:
        for i in range(len(fb)):
            fb[i] = 0
        driver.writefrom(fb)
        server.stop()

animate()

