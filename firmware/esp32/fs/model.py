
import uarray

def load_json(name):
    import config
    config.load()
    return config.load_json(config.model_dir + '/' + name)

class Model:
    def __init__(self, n_leds, positions, normals, inside, remap=None):
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

    gc.collect()

    return Model(n_leds, positions, normals, inside, remap)
