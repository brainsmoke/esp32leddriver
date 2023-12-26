
import uarray, math, cmath, random
import cball

import util

class Topo:

    def __init__(self, leds, tmp16=None, config=None, **kwargs):
        self.groups = tuple( bytearray(leds.groups[k]) for k in sorted(leds.groups.keys()) )
        self.set_topo(0)

        max_groups = max(len(g) for g in self.groups)
        max_colors = max_groups//3

        self.colors = [ cball.ColorDrift(256, 1) for i in range(max_colors) ]
        self.spots = [ -1 for _ in range(max_groups) ]
        self.occupied = [ False for _ in range(max_groups) ]

        phi = (1.+5**.5)/2.
        self.color_start = [ int(256*i/phi)%256 for i in range(max_colors) ]

        self.active = 0
        self.phase = 0

        self.palette = uarray.array('H', (0 for _ in range(max_groups*3)))
        self.fbuf_blend = uarray.array('H', (0 for _ in range(leds.n_leds * 3)) )

        self.set_lights(max_colors//3)
        if config:
            config.add_slider('topo', 0, len(self.groups)-1, 1, self.get_topo, self.set_topo, caption="topology")
            config.add_slider('lights',0, max_colors, 1, self.get_lights, self.set_lights, caption="lights")

    def set_topo(self,ix):
        self.topo = ix%len(self.groups)

    def get_topo(self):
        return self.topo
 
    def try_allocate_random_spot(self, ix):
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
        for i,v in enumerate(self.color_start):
            if self.phase == v:
                 self.free_spot(i)
                 if i < self.n_colors:
                     self.try_allocate_random_spot(i)

        for i in range(len(self.spots)):
            ix = self.spots[i]
            if ix != -1:
                self.colors[i].next_color_into(self.palette, ix*3)

        cball.apply_palette(out, self.groups[self.topo], self.palette)
        cball.array_blend(self.fbuf_blend, self.fbuf_blend, out, .03)
        cball.array_copy(out, self.fbuf_blend)

        self.phase += 1
        self.phase %= 256


