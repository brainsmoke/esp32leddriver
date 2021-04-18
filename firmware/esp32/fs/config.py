
essid, password = None, None

use_tls, key_file, cert_file = False, None, None

uart_baudrate, uart_rx, uart_tx = 1000000, None, None

model_dir = "/conf"

_loaded = False

def load_json(filename):

    import uio, ujson, gc
    config = None

    try:
        with uio.open(filename) as f:
            config = ujson.load(f)
    except OSError:
        print ("meh.")
        pass

    gc.collect()

    return config


def reload():
    import gc

    global essid, password, port, uart_baudrate, uart_rx, uart_tx, model_dir, use_tls, key_file, cert_file

    config = load_json("/secret/network.json")

    if config != None:
        if 'wifi' in config and 'essid' in config['wifi'] and 'password' in config['wifi']:
            essid, password = config['wifi']['essid'], config['wifi']['password']

    del config
    gc.collect()

    config = load_json("/secret/httpd.json")

    if config != None:
        if 'use_tls' in config:
            use_tls = bool(config['use_tls'])
        if 'tls' in config and 'key' in config['tls']:
            key_file = str(config['tls']['key'])
        if 'tls' in config and 'cert' in config['tls']:
            cert_file = str(config['tls']['cert'])

    del config
    gc.collect()

    config = load_json("/conf/board.json")

    if config != None:
        if 'uart' in config:
            if 'baudrate' in config['uart']:
                uart_baudrate = config['uart']['baudrate']
            if 'rx' in config['uart']:
                uart_rx = int(config['uart']['rx'])
            if 'tx' in config['uart']:
                uart_tx = int(config['uart']['tx'])

    del config
    gc.collect()

    config = load_json("/conf/model.json")

    if config != None:
        if 'model' in config:
            model_dir = config['model']

    del config
    gc.collect()

def load():
    global _loaded
    if not _loaded:
        reload()
        _loaded = True


