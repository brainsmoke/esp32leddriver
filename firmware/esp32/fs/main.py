
import machine, utime, gc, re

import config, model, uartpixel, cball, configform, csrf, wifi
from conf.load import get_animations, get_config_animation

import esp
#esp.osdebug(None)
import _thread
_thread.stack_size(8192)

import machine
machine.freq(240000000)

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

wifi.connect(wait=False)

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

for caption, Ani, settings in get_animations():
    name = caption.lower()
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
webdir.add_webdir(server, '/css', '/webroot/css', index=True, compression='gzip', cache_control="max-age=86400")

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

from button import button, button_timeout_wait

def wait_for_interrupt():
    while True:
        utime.sleep(60)

if config.button_next:
    button(config.button_next, on_down=player.next, pull=machine.Pin.PULL_UP)

if config.button_previous:
    button(config.button_previous, on_down=player.previous, pull=machine.Pin.PULL_UP)

# normal mode
try:
    player.on()
    server.start()
    gc.collect()
    if not wifi.fallback():
        if wifi.is_failsafe_configured():
            button_timeout_wait(0, timeout=1000)
        else:
            wait_for_interrupt()

    print("[dropping to failsafe mode]")
    info = wifi.info()
    wifi.connect_ap(wait=True)
    caption, Ani, settings = get_config_animation()
    name = caption.lower()
    player.add_animation( name, Ani( leds, tmpfloat=tmpfloat, tmp16=tmp16, config=None, **settings) )
    import admin

    def reset():
        wifi.disconnect()
        machine.reset()

    admin.get_form(info, reset_func=reset, form=cur_animation.add_group(name, caption=caption))
    while True:
        utime.sleep(1000) # lock with no load
finally:
    player.stop()
    try:
        server.stop()
    except OSError as e:
        print(e)

