
import uarray, math
import cball

class Fire:

    def calc_ca_map(self):
        factor_x, exponent = self.factor/512**1.25, self.exponent
        for i in range(512):
            self.ca_map[i] = max(0, min(255, int(factor_x * i**exponent)))

    def __init__(self, leds, config=None, **kwargs):
        import model

        self.factor = 233
        self.exponent = 1.25
        self.initial_a = 0
        self.initial_b = 56
        self.ca_map = bytearray(512)
        self.calc_ca_map()

        json = model.load_json("fire.json")
        self.width = int(json['width'])
        self.height = int(json['height'])
        self.voronoi_map = uarray.array('H', json['voronoi'])

        self.cells = bytearray(self.width * (self.height+1))
        self.fbuf_2d = uarray.array('H', (0 for _ in range(self.width * self.height * 3)) )
        self.palette = uarray.array('H', (0 for _ in range(64 * 3)) )
        self.fbuf_blend = uarray.array('H', (0 for _ in range(leds.n_leds * 3)) )

        for i in range(0,len(self.palette)/3):
            self.palette[i*3]   = 0x101*min(255, i*16)
            self.palette[i*3+1] = 0x0c1*min(255, int(1024*((i/64.)**1.5)) )
            self.palette[i*3+2] = 0x101*min(255, (i*i)//32)

        self.latt_weight = uarray.array('f', (math.sin(math.pi*(x+.5)/self.height) for x in range(self.height)))
        self.max_weight = uarray.array('f', (0. for _ in range(leds.n_leds)))
        for y in range(self.height):
            for x in range(self.width):
                self.max_weight[self.voronoi_map[x+y*self.width]] += self.latt_weight[y]

        for i in range(len(self.max_weight)):
            self.max_weight[i] = 1./self.max_weight[i]

        self.tmp_float_buf = uarray.array('f', (0. for _ in range(leds.n_leds*3)))
        self.phase = 0
        self.set_speed(25)
        if config:
            config.add_slider('speed', 0, 50, 1, self.get_speed, self.set_speed, caption="speed")
            config.add_slider('factor', 100, 400, 1, self.get_factor, self.set_factor, caption="factor")
            config.add_slider('exponent', 1., 1.5, .005, self.get_exponent, self.set_exponent, caption="exponent")
            config.add_slider('initial_a', 0, 63, 1, self.get_initial_a, self.set_initial_a, caption="seed a")
            config.add_slider('initial_b', 0, 63, 1, self.get_initial_b, self.set_initial_b, caption="seed b")

    def set_factor(self, factor):
        self.factor = factor
        self.calc_ca_map()

    def get_factor(self):
        return self.factor

    def set_exponent(self, exponent):
        self.exponent = exponent
        self.calc_ca_map()

    def get_exponent(self):
        return self.exponent

    def set_initial_a(self, initial_a):
        self.initial_a = initial_a
        self.calc_ca_map()

    def get_initial_a(self):
        return self.initial_a

    def set_initial_b(self, initial_b):
        self.initial_b = initial_b
        self.calc_ca_map()

    def get_initial_b(self):
        return self.initial_b

    def set_speed(self, speed):
        self.speed = speed
        steps_per_frame = speed/100.
        self.speed_f = 1-0.75**steps_per_frame

    def get_speed(self):
        return self.speed

    def next_frame(self, fbuf):
        self.phase = self.phase+self.speed
        if self.phase >= 100:
            cball.ca_update(self.cells, self.width, self.height+1, self.ca_map, self.initial_a, self.initial_b)
        self.phase %= 100
        cball.apply_palette(self.fbuf_2d, self.cells, self.palette)
        cball.latt_long_map(fbuf, self.fbuf_2d, self.voronoi_map, self.latt_weight, self.max_weight, self.tmp_float_buf)
        cball.array_blend(self.fbuf_blend, self.fbuf_blend, fbuf, self.speed_f)
        cball.array_copy(fbuf, self.fbuf_blend)

