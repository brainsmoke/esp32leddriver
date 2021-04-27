
import machine, network, utime, gc, re

import config, model, uartpixel, cball, configform

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


form = configform.ConfigTree("/player", tag="main")

from player import Player

player = Player(driver, leds)

form.add_action('previous', player.previous, player.is_on,  caption="&lt;&lt;" )
form.add_action('off',      player.off,      player.is_on,  caption="Off"      )
form.add_action('on',       player.on,       player.is_off, caption="On"       )
form.add_action('next',     player.next,     player.is_on,  caption="&gt;&gt;" )

select = form.add_select_group('ani', player.get_selected)

form.add_slider('brightness', 0.01, 1, .01, player.get_brightness, player.set_brightness, caption="brightness" )
form.add_slider('gamma',         1, 4, .1,  player.get_gamma,      player.set_gamma,      caption="gamma"      )

for Ani in (Lorenz, Fire, Gradient, Orbit, Spiral, Wobble):
    name = Ani.__name__.lower()
    caption = Ani.__name__
    print (name)
    player.add_animation( name, Ani( leds, select.add_group(name, caption=caption) ) )

from esphttpd import HTTP_Server, redirect, urldecode

if config.use_tls:
    server = HTTP_Server(use_tls=True, keyfile=config.key_file, certfile=config.cert_file) # https
else:
    server = HTTP_Server()

#import webdir
#webdir.add_webdir(server, '/script', '/webroot/script', index=True, compression='gzip')
#webdir.add_webdir(server, '/css', '/webroot/css', index=True, compression='gzip')
#webdir.add_webdir(server, '/models', '/models', index=True)

#form.print()

@server.route("/", "GET")
@server.buffered()
def index(req, out):
    out.write("""<!DOCTYPE html><html><head>
<link rel="icon" href="data:,">
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1">
<style type="text/css">
body
{
    font-family: Sans;
}

mainmeh
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

h1,h4
{
    text-align: center;
}

h4
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
""")
    form.html(out)

@server.route("/player/*", "POST")
def handler(req):
    path = urldecode(req.get_path()).decode('utf-8')
    form.set(path[7:], get_val(req))
    redirect(req, "/")

try:
    # quick hack :-P
    arr = bytearray(256)
    regex = re.compile('(^|&)value=([^&]*)(&|$)')
    def get_val(req):
        n = req.recv(arr)
        g=regex.search(bytes(arr[:n]))
        if g:
            value = urldecode(g.group(2)).decode('utf-8')
        else:
            value = ''
        return value

    player.on()
    server.start()
    gc.collect()
    player.run()
finally:
    server.stop()

