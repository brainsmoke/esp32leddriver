
import uarray, random
import cball
import util

class ShadowPlay:

    def __init__(self, leds, config=None, **kwargs):

        self.n_colors = min(6, leds.n_leds)
        max_colors = min(32, leds.n_leds)
        self.colors = [ cball.ColorDrift(512, 1) for i in range(max_colors) ]
        self.mapping = tuple( i*3 for i, x in enumerate(leds.inside) if x )
        self.occupied = [ False for _ in self.mapping ]
        phi = (1.+5**.5)/2.
        self.color_start = [ int(512*i/phi)%512 for i in range(max_colors) ]
        self.active = 0
        self.phase = 0
        self.spots = [ -1 for _ in range(max_colors) ]

        if config:
            config.add_slider('lights',1, 32, 1, self.get_lights, self.set_lights, caption="lights")

    def try_allocate_random_spot(self, ix):
        if self.active >= len(self.occupied):
            return False

        free_spaces = len(self.occupied)-self.active
        n = random.randrange(0, free_spaces)
        for i, v in enumerate(self.occupied):
            if v:
                continue
            if n == 0:
                self.occupied[i] = True
                self.active += 1
                self.spots[ix] = i
                return
            n -= 1
        assert False

    def free_spot(self, ix):
        n = self.spots[ix]
        self.spots[ix] = -1
        if 0 <= n < len(self.occupied) and self.occupied[n]:
            self.occupied[n] = False
            self.active -= 1

    def set_lights(self, lights):
        self.n_colors = lights

    def get_lights(self):
        return self.n_colors

    @micropython.native
    def next_frame(self, out):
        cball.array_set(out, 0)
        for i,v in enumerate(self.color_start):
            if self.phase == v:
                 self.free_spot(i)
                 if i < self.n_colors:
                     self.try_allocate_random_spot(i)

        for i in range(len(self.spots)):
            ix = self.spots[i]
            if ix != -1:
                self.colors[i].next_color_into(out, self.mapping[ix])

        self.phase += 1
        self.phase %= 512

