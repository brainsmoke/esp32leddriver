
essid, password, port = None, None, 2029

uart_baudrate, uart_rx, uart_tx = 1000000, None, None

_loaded = False

def reload():
    import uio, ujson, gc

    global essid, password, port, uart_baudrate, uart_rx, uart_tx

    with uio.open("/conf/network.json") as f:
        config = ujson.load(f)

    gc.collect()

    if 'wifi' in config and 'essid' in config['wifi'] and 'password' in config['wifi']:
        essid, password = config['wifi']['essid'], config['wifi']['password']

    if 'socket' in config and 'port' in config['socket']:
        port = int(config['socket']['port'])

    del config

    gc.collect()

    with uio.open("/conf/board.json") as f:
        config = ujson.load(f)

    gc.collect()

    if 'uart' in config:
        if 'baudrate' in config['uart']:
            uart_baudrate = config['uart']['baudrate']
        if 'rx' in config['uart']:
            uart_rx = int(config['uart']['rx'])
        if 'tx' in config['uart']:
            uart_tx = int(config['uart']['tx'])

    del config

    gc.collect()

def load():
    global _loaded
    if not _loaded:
        reload()
        _loaded = True


