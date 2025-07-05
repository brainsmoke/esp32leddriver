
import network, utime

import config

wlan = network.WLAN(network.STA_IF)
ap = network.WLAN(network.AP_IF)
ap.active(False)
cur = wlan

def info():
    mac = ':'.join('{:02x}'.format(b) for b in wlan.config('mac') )
    return tuple( zip( ("MAC", "IP", "Subnet", "Gateway", "DNS" ),
                        (mac, ) + wlan.ifconfig() ) )

def is_configured():
    return config.essid != None

def is_failsafe_configured():
    return config.failsafe_essid != None

def is_connected():
    return cur.isconnected()

def fallback():
    return is_failsafe_configured() and not is_connected() and config.failsafe_auto_fallback

def wait_for_connection(verbose=True):
    try:
        for i in range(20):
           if cur.isconnected():
               if verbose:
                   print(cur.ifconfig())
               return True
           utime.sleep(.5)
    except OSError:
        pass

    cur.active(False)
    return False

def connect_client(wait=True):
    if not is_configured():
        wlan.active(False)
        return

    try:
        ap.active(False)
        wlan.active(True)
        if wlan.isconnected():
            wlan.disconnect()

        print('connecting to network...')
        wlan.connect(config.essid, config.password)
        if wait:
            wait_for_connection()
    except OSError:
        wlan.active(False)
        pass

def connect_ap(wait=True):
    wlan.disconnect()
    wlan.active(False)
    if not is_failsafe_configured():
        return
    essid, password, ip = config.failsafe_essid, config.failsafe_password, config.failsafe_ip
    utime.sleep(.5)
    ap.active(True)
    print([essid, password, ip])
    ap.config(essid=essid, password=password, authmode=network.AUTH_WPA2_PSK, max_clients=4)
    ap.ifconfig( [ip, '255.255.255.0', ip, ip] )
    print(ap.ifconfig())
    cur=ap

def get_networks():
    networks = set()
    pre_state = wlan.active()
    wlan.active(True)

    for x in wlan.scan():
        try:
            networks.add(x[0].decode('utf-8'))
        except UnicodeError:
            pass
    networks.discard('')
    wlan.active(pre_state)
    return list(networks)


def connect(wait=True):
    if is_configured():
        connect_client(wait)

def disconnect():
    ap.active(False)
    wlan.active(False)
