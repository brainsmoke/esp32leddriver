
import network, utime

import config

wlan = network.WLAN(network.STA_IF)
ap = network.WLAN(network.AP_IF)
ap.active(False)
cur = wlan

def info(network=wlan):
    if network.isconnected():
        mac = ':'.join('{:02x}'.format(b) for b in network.config('mac') )
        return tuple( zip( ("MAC", "IP", "Subnet", "Gateway", "DNS" ),
                            (mac, ) + network.ifconfig() ) )
    else:
        return None

def print_info(network=wlan):
    nw_conf = info(network)
    if nw_conf is not None:
        print("\nNetwork:")
        for desc, val in nw_conf:
            print("  {}: {}".format(desc, val))
        print("")

def is_configured():
    return config.essid != None

def is_failsafe_configured():
    return config.failsafe_essid != None

def is_connected():
    return cur.isconnected()

def fallback():
    return is_failsafe_configured() and not is_connected() and config.failsafe_auto_fallback

def wait_for_connection(verbose=True, n_secs=10):
    try:
        for i in range(2*n_secs):
           if cur.isconnected():
               if verbose:
                   print_info(cur)
               return True
           utime.sleep(.5)
    except OSError:
        pass

    cur.active(False)
    return False

def connect_client(wait=True, n_secs=10):
    if not is_configured():
        wlan.active(False)
        return

    try:
        wlan.active(True)
        if wlan.isconnected():
            wlan.disconnect()

        print('connecting to network...')
        wlan.connect(config.essid, config.password)
        if wait:
            wait_for_connection(n_secs=n_secs)
    except OSError:
        wlan.active(False)
        pass

def connect_ap(wait=True):
    if wlan.isconnected():
        wlan.disconnect()
    wlan.active(False)
    if not is_failsafe_configured():
        return
    essid, password, ip = config.failsafe_essid, config.failsafe_password, config.failsafe_ip
    utime.sleep(.5)
    ap.active(True)
    print("\n  ESSID: {}\n  Password: {}\n  IP: {}\n".format(repr(essid), repr(password), ip))
    ap.config(essid=essid, password=password, authmode=network.AUTH_WPA2_PSK, max_clients=4)
    ap.ifconfig( [ip, '255.255.255.0', ip, ip] )
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


def connect(wait=True, n_secs=10):
    if is_configured():
        connect_client(wait, n_secs)

def disconnect():
    ap.active(False)
    wlan.active(False)

def test_connection():
    connect(n_secs=5)
    _info = info()
    wlan.active(False)
    return _info
