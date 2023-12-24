
essid, password = None, None

failsafe_essid, failsafe_password, failsafe_ip = None, None, None

use_tls, key_file, cert_file = False, None, None

api_key = None

uart_baudrate, uart_rx, uart_tx = 1000000, None, None

pulse_width, pulse_width_max, pulse_width_min = None, None, None

model_dir = "/conf"
led_order = "GRB"
cutoff = 24
fps = 60

_loaded = False

def load_json(filename):

    import uio, ujson, gc
    config = None

    try:
        with uio.open(filename) as f:
            config = ujson.load(f)
    except (OSError, ValueError):
        pass

    gc.collect()

    return config

def reload_network():
    global essid, password
    config = load_json("/secret/network.json")

    essid = None
    password = None
    if config != None:
        if 'wifi' in config and 'essid' in config['wifi']:
            essid = config['wifi']['essid']
            if 'password' in config['wifi']:
                password = config['wifi']['password']
            else:
                password = None

    del config

def reload_failsafe():
    global failsafe_essid, failsafe_password, failsafe_ip
    config = load_json("/secret/failsafe.json")

    if config != None:
        if 'wifi' in config and \
           'essid' in config['wifi'] and \
           'password' in config['wifi'] and \
           'network' in config and \
           'ip' in config['network']:
            failsafe_essid = config['wifi']['essid']
            failsafe_password = config['wifi']['password']
            failsafe_ip = config['network']['ip']

    del config

def write_network_conf(essid, password):

    import ujson
    with open("/secret/network.json", "w") as f:
        settings = {}
        if essid != None:
            settings['essid'] = essid
        if password != None:
            settings['password'] = password

        ujson.dump( { 'wifi' : settings }, f )

    reload_network()

def write_failsafe_conf(essid, password, ip):

    import ujson
    with open("/secret/failsafe.json", "w") as f:
        ujson.dump( {
            'wifi' : { 'essid': str(essid), 'password': str(password) },
            'network' : { 'ip': str(ip) },
        }, f)

    reload_failsafe()

def reload():
    import gc

    global port, uart_baudrate, uart_rx, uart_tx, model_dir, led_order, cutoff, fps, use_tls, key_file, cert_file, api_key
    global pulse_width, pulse_width_min, pulse_width_max
    reload_network()
    reload_failsafe()
    gc.collect()

    config = load_json("/secret/httpd.json")

    if config != None:
        use_tls = bool(config.get('use_tls', False))

        if 'tls' in config and 'key' in config['tls']:
            key_file = str(config['tls']['key'])
        if 'tls' in config and 'cert' in config['tls']:
            cert_file = str(config['tls']['cert'])

        if config.get('use_api_key') and 'api_key' in config:
            api_key = str(config['api_key'])

    del config
    gc.collect()

    config = load_json("/conf/hardware.json")

    if config != None:
        if 'uart' in config:
            if 'baudrate' in config['uart']:
                uart_baudrate = config['uart']['baudrate']
            if 'rx' in config['uart']:
                uart_rx = int(config['uart']['rx'])
            if 'tx' in config['uart']:
                uart_tx = int(config['uart']['tx'])
        if 'driver' in config:
            if 'pulse_width' in config['driver']:
                pulse_width = int(config['driver']['pulse_width'])
            if 'pulse_width_min' in config['driver']:
                pulse_width_min = int(config['driver']['pulse_width_min'])
            if 'pulse_width_max' in config['driver']:
                pulse_width_max = int(config['driver']['pulse_width_max'])
        if 'model' in config:
            model_dir = config['model']
        if 'led_order' in config:
            led_order = config['led_order'].upper()
        if 'cutoff' in config:
            cutoff = int(config['cutoff'])
        if 'fps' in config:
            fps = int(config['fps'])

    del config
    gc.collect()

def load():
    global _loaded
    if not _loaded:
        reload()
        _loaded = True


