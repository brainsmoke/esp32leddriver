
import uarray

class Model:
    def __init__(self, n_leds, positions, normals, inside):
        self.n_leds = n_leds
        self.positions = positions
        self.normals = normals
        self.flat_data = uarray.array('f')
        for p, n in zip(positions, normals):
            for x in p:
                self.flat_data.append(x)
            for x in n:
                self.flat_data.append(x)

        self.inside = inside

def load(filename):
    import uio, ujson, gc

    with uio.open(filename) as f:
        json = ujson.load(f)

    gc.collect()

    n_leds    = len(json['leds'])
    positions = tuple( tuple(led['position']) for led in json['leds'] )
    normals   = tuple( tuple(led['normal'])   for led in json['leds'] )
    inside    = tuple( led['inside']          for led in json['leds'] )

    del json

    gc.collect()

    return Model(n_leds, positions, normals, inside)
