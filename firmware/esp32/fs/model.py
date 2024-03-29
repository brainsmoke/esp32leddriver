
import uarray

import config

def load_json(name):
    config.load()
    return config.load_json(config.model_dir + '/' + name)

def open_file(name, *args, **kwargs):
    config.load()
    return open(config.model_dir + '/' + name, *args, **kwargs)

class Model:
    def __init__(self, n_leds, positions, normals, inside, remap=None, circuits={}, groups={}):
        self.n_leds = n_leds
        self.positions = positions
        self.normals = normals
        self.flat_data = uarray.array('f')
        for p, n in zip(positions, normals):
            for x in p+n:
                self.flat_data.append(x)

        if remap == None:
            self.remap = None
        else:
            self.remap = uarray.array('H', remap )

        self.circuits = circuits
        self.groups = groups

        self.inside = inside

def load():

    import gc
    json = load_json('leds.json')

    n_leds    = len(json['leds'])
    positions = tuple( tuple(led['position']) for led in json['leds'] )
    normals   = tuple( tuple(led['normal'])   for led in json['leds'] )
    inside    = tuple( led['inside']          for led in json['leds'] )

    del json

    gc.collect()

    remap = load_json('remap.json')
    circuits = load_json('circuits.json')

    if circuits == None:
        circuits = {}

    groups = load_json('groups.json')

    if groups == None:
        groups = {}

    gc.collect()

    return Model(n_leds, positions, normals, inside, remap, circuits, groups)
