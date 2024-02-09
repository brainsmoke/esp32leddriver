
import machine, network, utime, gc, re

import config, model, uartpixel, cball, configform, csrf
from conf.load import get_animations

from ani.gradient import ConfigMode

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
                             led_order= config.led_order,
                             brightness = config.brightness,
                             cutoff   = config.cutoff,
                             remap    = leds.remap,
                             fps      = config.fps,
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

tmpfloat = driver.create_framebuffer_float()
tmp16 = driver.create_framebuffer16()

player.start()

for Ani, settings in get_animations():
    name = Ani.__name__.lower()
    caption = Ani.__name__
    try:
        print (caption)
        player.add_animation( name, Ani( leds, tmpfloat=tmpfloat, tmp16=tmp16, config=cur_animation.add_group(name, caption=caption), **settings) )
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

import webdir
#webdir.add_webdir(server, '/script', '/webroot/script', index=True, compression='gzip')
webdir.add_webdir(server, '/css', '/webroot/css', index=True, compression='gzip', cache_control="max-age=86400")
#webdir.add_webdir(server, '/models', '/models', index=True)

#form.print()

web_header = '''<!DOCTYPE html><html><head>
<link rel="icon" href="data:,">
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" type="text/css" href="/css/style.css?boot=''' + csrf.get_boot_token() + '''">
<body>'''

@server.route("/", "GET")
@server.buffered
def index(req, out):
    out.write(web_header)
    #print ( "server ({}:{}) client ({}:{}) protocol ({})".format(
    #        req.get_server_host(), req.get_server_port(),
    #        req.get_remote_host(), req.get_remote_port(),
    #        req.get_protocol() ) )
    csrf_tag = b'<input type="hidden" name="csrf" value="'+csrf.get_csrf_token( req )+b'" />'
    form.html(out, csrf_tag=csrf_tag)

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

def wait_for_interrupt():
    while True:
        utime.sleep(60)

# normal mode
try:
    player.on()
    server.start()
    gc.collect()
    if config.failsafe_essid == None:
        wait_for_interrupt()
    else:
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

