
import config

def query(prompt, default=None):
    if default == None:
        default = ''

    s = input(prompt + " ["+str(default)+"]: ")
    if s == '':
        return default
    else:
        return s

def setup_interactive():

    essid = query("wifi essid", default=config.essid)
    if essid == '':
        raise ValueError("no essid given")

    password = query("wifi password", default=config.password)
    if password == '':
        raise ValueError("no password given")

    config.write_network_conf(essid, password)


def setup_open_interactive():

    essid = query("wifi essid", default=config.essid)
    if essid == '':
        raise ValueError("no essid given")

    config.write_network_conf(essid, None)


def failsafe_interactive():

    old_essid = config.failsafe_essid
    if old_essid == None:
        old_essid = 'greatcircles'

    essid = query("appear as access point with essid", default=old_essid)
    if essid == '':
        raise ValueError("no essid given")

    password = query("wifi password (min 8 characters)", default=config.failsafe_password)
    if len(password) < 8:
        raise ValueError("bad password")

    old_ip = config.failsafe_ip
    if old_ip == None:
        old_ip = '10.0.0.1'

    ip = query("IP", default=old_ip)

    if config.failsafe_auto_fallback:
        fallback = "yes"
    else:
        fallback = "no"

    auto_fall_back = ''
    while auto_fall_back not in ("yes", "no"):
        auto_fall_back = query("automatically fall back to failsafe mode if network connection fails on boot", default=fallback)

    auto_fall_back = auto_fall_back == "yes"

    config.write_failsafe_conf(essid, password, ip, auto_fall_back)


