
import machine, network, utime, uarray, gc, re

import config, model, uartpixel, cball, configform, csrf

from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble, ConfigMode
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import Checkers, AlienPlanet

import esp
#esp.osdebug(None)
import _thread
_thread.stack_size(8192)

import machine
machine.freq(240000000)

wlan = network.WLAN(network.STA_IF)
ap = network.WLAN(network.AP_IF)
ap.active(False)

def wifi_connect(essid, password, wait=True):
    try:
        wlan.active(True)
        if wlan.isconnected():
            wlan.disconnect()

        print('connecting to network...')
        wlan.connect(essid, password)
        if wait:
            wait_for_wifi()
    except OSError:
        pass

def wifi_configure_ap(essid, password, ip):
    wlan.active(False)
    ap.active(True)
    print([essid, password, ip])
    ap.config(essid=essid, password=password, authmode=network.AUTH_WPA2_PSK, max_clients=4)
    ap.ifconfig( [ip, '255.255.255.0', ip, ip] )
    print(ap.ifconfig())

def wait_for_wifi(verbose=True):
    try:
        for i in range(20):
           if wlan.isconnected():
               if verbose:
                   print(wlan.ifconfig())
               break
           utime.sleep(.5)
    except OSError:
        pass

config.load()

def setup():
    from setup import setup_interactive
    setup_interactive()

def setup_open():
    from setup import setup_open_interactive
    setup_open_interactive()

def failsafe():
    from setup import failsafe_interactive
    failsafe_interactive()

if config.essid != None:
    wifi_connect(config.essid, config.password, wait=False)

leds = model.load()

driver = uartpixel.UartPixel(baudrate = config.uart_baudrate,
                             rx       = config.uart_rx,
                             tx       = config.uart_tx,
                             n        = leds.n_leds,
                             remap    = leds.remap,
                             framebuf = False )

if config.pulse_width != None:
    driver.set_pulse_width(config.pulse_width)

form = configform.ConfigRoot("/")

from player import Player

player = Player(driver)

form.add_action('previous', player.previous, player.is_on,  caption="&lt;&lt;" )
form.add_action('off',      player.off,      player.is_on,  caption="Off"      )
form.add_action('on',       player.on,       player.is_off, caption="On"       )
form.add_action('next',     player.next,     player.is_on,  caption="&gt;&gt;" )

cur_animation = form.add_select_group('ani', player.get_selected)

form.add_slider('brightness', 0.01, 1, .01, player.get_brightness, player.set_brightness, caption="brightness" )
form.add_slider('gamma',         1, 4, .1,  player.get_gamma,      player.set_gamma,      caption="gamma"      )

tmpfloat = uarray.array('f', 0 for _ in range(leds.n_leds * 3))
tmp16 = uarray.array('H', 0 for _ in range(leds.n_leds * 3))

player.start()

for Ani in (Lorenz, Rutherford, Fire, Gradient, Orbit, Wobble, Checkers, AlienPlanet, Spots, Chroma):
    name = Ani.__name__.lower()
    caption = Ani.__name__
    try:
        print (caption)
        player.add_animation( name, Ani( leds, tmpfloat=tmpfloat, tmp16=tmp16, config=cur_animation.add_group(name, caption=caption) ) )
    except KeyboardInterrupt as err:
        raise err
    except Exception as err:
        print ("failed loading {}: {}".format(caption, err))
    if player.is_off():
        player.on()


debug=False
def gc_test():
    print ("gc start")
    gc.collect()
    print ("gc end")

if debug:
    form.add_action('gc', gc_test, lambda : debug, caption="garbage collect" )

from esphttpd import HTTP_Server, redirect, urldecode, parse_formdata

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

dl { display: flex; flex-wrap: wrap; }
dt { width: 30%;margin:1em 0 0 0;}
dd { width: 69%;margin:1em 0 0 0;}
dd input { width:100% }
.network input[type=submit]{width: 30%; font-size: 100%;margin-bottom: 1em}

.color, .slider
{
    display: flex;
    flex-wrap: wrap;
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

.slider  input[type=submit],
.color  input[type=submit]
{
    width: 4em;
    font-size: 100%;
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
    flex-grow: 1;
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
    formdata = parse_formdata( req )
    token = formdata['csrf'].encode('utf-8')
    if csrf.verify_csrf_token(req, token) or csrf.verify_api_key(req, config.api_key):
        form.set(path, formdata)
    elif token != b'':
        print("[csrf verify failed!], token = {}".format(repr(token)))
    redirect(req, "/")

from gpiowait import PinEvent

event = PinEvent(0, trigger=machine.Pin.IRQ_FALLING)
def wait_for_buttonpress():
    while True:
        event.wait()
        utime.sleep(1) # don't make screen attaches trigger button presses
        if event.pin() == 0:
            break

# normal mode
try:
    player.on()
    server.start()
    gc.collect()
    wait_for_buttonpress()
finally:
    player.stop()
    try:
        server.stop()
    except OSError as e:
        print(e)

# failsafe mode
try:
    print("[dropping to failsafe mode]")
    mac = ':'.join('{:02x}'.format(b) for b in wlan.config('mac') )
    info = tuple( zip( ("MAC", "IP", "Subnet", "Gateway", "DNS" ),
                        (mac, ) + wlan.ifconfig() ) )
    if wlan.active():
        wlan.disconnect()
    configani = ConfigMode(leds)
    utime.sleep(.5)
    wifi_configure_ap(config.failsafe_essid, config.failsafe_password, config.failsafe_ip)
    player.set_animation(configani)
    player.start()
    import admin

    def reset():
        ap.active(False)
        machine.reset()

    form = admin.get_form( info, reset_func=reset )
    server.use_tls(False)
    server.start()
    while True:
        utime.sleep(1000) # lock with no load
finally:
    player.stop()
    try:
        server.stop()
    except OSError as e:
        print(e)

