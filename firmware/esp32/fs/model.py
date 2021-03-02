
import uarray

def load_json(name):
    import config
    config.load()
    return config.load_json(config.model_dir + '/' + name)

class Model:
    def __init__(self, n_leds, positions, normals, inside):
        self.n_leds = n_leds
        self.positions = positions
        self.normals = normals
        self.flat_data = uarray.array('f')
        for p, n in zip(positions, normals):
            self.flat_data.extend(p)
            self.flat_data.extend(n)

        self.inside = inside

def load():

    import gc
    json = load_json('/leds.json')

    n_leds    = len(json['leds'])
    positions = tuple( tuple(led['position']) for led in json['leds'] )
    normals   = tuple( tuple(led['normal'])   for led in json['leds'] )
    inside    = tuple( led['inside']          for led in json['leds'] )

    del json

    gc.collect()

    return Model(n_leds, positions, normals, inside)
