
import uarray, math
import cball

class Fire2D:

    def calc_ca_map(self):
        factor_x, exponent = self.factor/512**1.25, self.exponent
        for i in range(512):
            self.ca_map[i] = max(0, min(255, int(factor_x * i**exponent)))

    def __init__(self, leds, config=None, dim=None, **kwargs):
        import model

        self.factor = 249
        self.exponent = 1.265
        self.initial_a = 8
        self.initial_b = 63
        self.ca_map = bytearray(512)
        self.calc_ca_map()

        self.width, self.height = dim

        self.cells = bytearray(self.width * (self.height+1))
        self.fbuf_2d = uarray.array('H', (0 for _ in range(self.width * self.height)) )
        self.palette = uarray.array('H', (0 for _ in range(len(self.ca_map))) )
        self.fbuf_blend = uarray.array('H', (0 for _ in range(leds.n_leds)) )

        for i in range(64):
            self.palette[i]   = (0xffff*i)//63

        self.tmp_float_buf = uarray.array('f', (0. for _ in range(leds.n_leds)))
        self.phase = 0
        self.set_speed(12)
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

    @micropython.native
    def next_frame(self, fbuf):
        self.phase = self.phase+self.speed
        if self.phase >= 100:
            cball.ca_update(self.cells, self.width, self.height+1, self.ca_map, self.initial_a, self.initial_b)
        self.phase %= 100
        for i in range(len(fbuf)):
            fbuf[i] = self.palette[self.cells[i]]
        cball.array_blend(self.fbuf_blend, self.fbuf_blend, fbuf, self.speed_f)
        cball.array_copy(fbuf, self.fbuf_blend)

