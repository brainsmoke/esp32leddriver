
import machine, network, utime, uarray, gc, re

import config, model, uartpixel, cball, configform, csrf

from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import Checkers, AlienPlanet

import esp
#esp.osdebug(None)

import machine
machine.freq(240000000)

wlan = None
def wifi_connect(essid, password, wait=True):
    global wlan
    try:
        wlan = network.WLAN(network.STA_IF)
        wlan.active(True)
        if wlan.isconnected():
            wlan.disconnect()

        print('connecting to network...')
        wlan.connect(essid, password)
        if wait:
            wait_for_wifi()
    except OSError:
        pass

def wait_for_wifi():
    try:
        for i in range(10):
           if wlan.isconnected():
               print(wlan.ifconfig())
               break
           utime.sleep(.5)
    except OSError:
        pass

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
    wifi_connect(config.essid, config.password, wait=False)

leds = model.load()

driver = uartpixel.UartPixel(baudrate = config.uart_baudrate,
                             rx       = config.uart_rx,
                             tx       = config.uart_tx,
                             n        = leds.n_leds,
                             remap    = leds.remap,
                             framebuf = False )

form = configform.ConfigRoot("/")

from player import Player

player = Player(driver, leds)

form.add_action('previous', player.previous, player.is_on,  caption="&lt;&lt;" )
form.add_action('off',      player.off,      player.is_on,  caption="Off"      )
form.add_action('on',       player.on,       player.is_off, caption="On"       )
form.add_action('next',     player.next,     player.is_on,  caption="&gt;&gt;" )

select = form.add_select_group('ani', player.get_selected)

form.add_slider('brightness', 0.01, 1, .01, player.get_brightness, player.set_brightness, caption="brightness" )
form.add_slider('gamma',         1, 4, .1,  player.get_gamma,      player.set_gamma,      caption="gamma"      )

tmpfloat = uarray.array('f', 0 for _ in range(leds.n_leds * 3))
tmp16 = uarray.array('H', 0 for _ in range(leds.n_leds * 3))

for Ani in (Lorenz, Rutherford, Fire, Gradient, Orbit, Wobble, Checkers, AlienPlanet, Spots, Chroma):
    name = Ani.__name__.lower()
    caption = Ani.__name__
    try:
        print (caption)
        player.add_animation( name, Ani( leds, tmpfloat=tmpfloat, tmp16=tmp16, config=select.add_group(name, caption=caption) ) )
    except KeyboardInterrupt as err:
        raise err
    except Exception as err:
        print ("failed loading {}: {}".format(caption, err))


debug=False
def gc_test():
    print ("gc start")
    gc.collect()
    print ("gc end")

if debug:
    form.add_action('gc', gc_test, lambda : debug, caption="garbage collect" )

from esphttpd import HTTP_Server, redirect, urldecode

if config.essid != None:
    wait_for_wifi()

if config.use_tls:
    server = HTTP_Server(use_tls=True, keyfile=config.key_file, certfile=config.cert_file) # https
else:
    server = HTTP_Server()

#import webdir
#webdir.add_webdir(server, '/script', '/webroot/script', index=True, compression='gzip')
#webdir.add_webdir(server, '/css', '/webroot/css', index=True, compression='gzip')
#webdir.add_webdir(server, '/models', '/models', index=True)

#form.print()

# quick hack :-P
arr = memoryview(bytearray(256))
value_regex = re.compile('(^|&)value=([^&]*)(&|$)')
token_regex = re.compile('(^|&)csrf=([^&]*)(&|$)')
def get_val(req):
    n = req.recv(arr)
    form_content = bytes(arr[:n])
    g=value_regex.search(form_content)
    if g:
        value = urldecode(g.group(2)).decode('utf-8')
    else:
        value = ''

    g=token_regex.search(form_content)
    if g:
        token = urldecode(g.group(2))
    else:
        token = b''

    return value, token


@server.route("/", "GET")
@server.buffered
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

main
{
    max-width: 30em;
    margin: .5em;
}

.group
{
    width: 100%;
}

main, .group
{
    display: flex;
    flex-wrap: wrap;
    margin-left: auto;
    margin-right: auto;
}

form[action="/on"]       > input,
form[action="/off"]      > input
{
    flex-grow: 3;
    width: auto;
}

form[action="/on"]       > input:disabled,
form[action="/off"]      > input:disabled
{
    display: none;
}

form[action="/next"]     > input,
form[action="/previous"] > input
{
    flex-grow: 2;
    width: auto;
}

.select_group, .action
{
    display: contents;
}

.color, .slider
{
    width: 100%;
}

input[type=color]
{
    height: 2em;
}

input[type=submit]
{
    height: 2em;
    font-size: 200%;
}

h2,h4,label
{
    text-align: center;
    width: 100%;
}

h4,label
{
    font-size: 14pt;
    margin-bottom: .2em;
    margin-top: .5em;
}

input
{
    width: 100%;
}
</style>
<body>
""")
    #print ( "server ({}:{}) client ({}:{}) protocol ({})".format(
    #        req.get_server_host(), req.get_server_port(),
    #        req.get_remote_host(), req.get_remote_port(),
    #        req.get_protocol() ) )
    csrf_tag = b'<input type="hidden" name="csrf" value="'+csrf.get_csrf_token( req )+b'" />'
    form.html(out, csrf_tag=csrf_tag)

#import uctypes,cball,binascii
#
#@server.route("/read", "POST")
#def handler(req):
#    value, token = get_val(req)
#    values = value.split(',')
#    addr = int(values[0], 16)
#    size = int(values[1], 16)
#    b = uctypes.bytes_at(addr, size)
#    req.write_all(binascii.hexlify(b))
#
#uint32 = { 'val': 0 | uctypes.UINT32 }
#@server.route("/write4", "POST")
#def handler(req):
#    value, token = get_val(req)
#    values = value.split(',')
#    addr = int(values[0], 16)
#    val = int(values[1], 16)
#    v = uctypes.struct(addr, uint32, uctypes.NATIVE)
#    v.val = val
#    req.write_all(hex(v.val))
#
#
#@server.route("/write", "POST")
#def handler(req):
#    value, token = get_val(req)
#    values = value.split(',')
#    addr = int(values[0], 16)
#    b = binascii.unhexlify(values[1])
#    ba = uctypes.bytearray_at(addr, len(b))
#    cball.bytearray_memcpy(ba, b)
#    b = uctypes.bytes_at(addr, len(b))
#    req.write_all(binascii.hexlify(b))

@server.route("/*", "POST")
def handler(req):
    path = urldecode(req.get_path()).decode('utf-8')
    value, token = get_val(req)
    if csrf.verify_csrf_token(req, token) or csrf.verify_api_key(req, config.api_key):
        form.set(path, value)
    elif token != b'':
        print("[csrf verify failed!], token = {}".format(repr(token)))
    redirect(req, "/")

try:
    player.on()
    server.start()
    gc.collect()
    player.run()
finally:
    server.stop()

