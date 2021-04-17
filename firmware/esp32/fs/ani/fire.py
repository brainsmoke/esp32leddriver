
import uarray, math
import cball

class Fire:

    def __init__(self, leds):
        import model

        self.ca_map = bytes(min(255, int(((i/64)**1.25)/3.7*64.)) for i in range(512))

        json = model.load_json("fire.json")
        self.width = int(json['width'])
        self.height = int(json['height'])
        self.voronoi_map = uarray.array('H', json['voronoi'])

        self.cells = bytearray(self.width * (self.height+1))
        self.fbuf_2d = bytearray(self.width * self.height * 3)
        self.palette = bytearray(64 * 3)
        self.fbuf_blend = bytearray(leds.n_leds * 3)

        for i in range(0,len(self.palette)/3):
            self.palette[i*3]   = min(255, i*12)
            self.palette[i*3+1] = min(255, int(1024*((i/64.)**1.5)) )
            self.palette[i*3+2] = min(255, (i*i)//64)

        self.latt_weight = uarray.array('f', math.sin(math.pi*(x+.5)/self.height) for x in range(self.height))
        self.max_weight = uarray.array('f', 0. for _ in range(leds.n_leds))
        for y in range(self.height):
            for x in range(self.width):
                self.max_weight[self.voronoi_map[x+y*self.width]] += self.latt_weight[y]

        for i in range(len(self.max_weight)):
            self.max_weight[i] = 1./self.max_weight[i]

        self.tmp_float_buf = uarray.array('f', 0. for _ in range(leds.n_leds*3))
        self.phase = 0
        self.phase_max = 4

    def next_frame(self, fbuf):
        if self.phase == 0:
            cball.ca_update(self.cells, self.width, self.height+1, self.ca_map)
        self.phase = (self.phase+1)%self.phase_max
        cball.apply_palette(self.fbuf_2d, self.cells, self.palette)
        cball.latt_long_map(fbuf, self.fbuf_2d, self.voronoi_map, self.latt_weight, self.max_weight, self.tmp_float_buf)
        cball.bytearray_blend(self.fbuf_blend, self.fbuf_blend, fbuf, 1/8)
        cball.bytearray_memcpy(fbuf, self.fbuf_blend)

