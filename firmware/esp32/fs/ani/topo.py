
import uarray, math, cmath
import cball

import util
from ani.gradient import BaseGradient

class Topo(BaseGradient):

    def __init__(self, leds, tmp16=None, config=None, **kwargs):
        self.colormap = bytearray( leds.n_leds )
        self.palette = uarray.array('H', (0,0,0) )
        self.colorfb  = tmp16
        if tmp16 != None:
            self.colorfb = tmp16
        else:
            self.colorfb = uarray.array('H', 0 for _ in range( leds.n_leds * 3 ) )
        self.color = cball.ColorDrift(2048, 2)

        self.wave = util.get_smooth_wave()
        self.mask = None
        n = len(self.wave)
        self.base_rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))
        self.rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.base_rotations[i*3  ] = int((n * (1+cmath.phase(complex(y,z)) / (math.pi*2))) % n)
            self.base_rotations[i*3+1] = int((n * (1+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.base_rotations[i*3+2] = int((n * (1+cmath.phase(complex(x,y)) / (math.pi*2))) % n)

        self.phase = 0
        self.phase_max = 4*7*8*9* 10
        self.speed = 4

        self.groups = tuple( leds.groups[k] for k in sorted(leds.groups.keys()) )
        self.set_rtopo(0)
        self.set_gtopo(1)
        self.set_btopo(2)

        if config:
            config.add_slider('speed', 0, 10, 1, self.get_speed, self.set_speed, caption="speed")
            config.add_slider('rtopo', 0, len(self.groups)-1, 1, self.get_rtopo, self.set_rtopo, caption="red topology")
            config.add_slider('gtopo', 0, len(self.groups)-1, 1, self.get_gtopo, self.set_gtopo, caption="green topology")
            config.add_slider('btopo', 0, len(self.groups)-1, 1, self.get_btopo, self.set_btopo, caption="blue topology")

    def set_rtopo(self,ix):
        self.rtopo = ix%len(self.groups)
        g = self.groups[self.rtopo]
        n = len(self.wave)
        phi = (1+5**.5)/2
        for i in range(len(g)):
            self.rotations[i*3] = (self.base_rotations[i*3] + int(phi*g[i]*n))%n
#            self.rotations[i*3] = (int(phi*g[i]*n))%n

    def get_rtopo(self):
        return self.rtopo

    def set_gtopo(self,ix):
        self.gtopo = ix%len(self.groups)
        g = self.groups[self.gtopo]
        n = len(self.wave)
        phi = (1+5**.5)/2
        for i in range(len(g)):
            self.rotations[i*3+1] = (self.base_rotations[i*3+1] + int(phi*g[i]*n))%n
#            self.rotations[i*3+1] = (int(phi*g[i]*n))%n

    def get_gtopo(self):
        return self.gtopo

    def set_btopo(self,ix):
        self.btopo = ix%len(self.groups)
        g = self.groups[self.btopo]
        n = len(self.wave)
        phi = (1+5**.5)/2
        for i in range(len(g)):
            self.rotations[i*3+2] = (self.base_rotations[i*3+2] + int(phi*g[i]*n))%n
#            self.rotations[i*3+2] = (int(phi*g[i]*n))%n

    def get_btopo(self):
        return self.btopo
 
    def next_frame(self, fbuf):
        super().next_frame(fbuf)
        self.color.next_color_into(self.palette, 0)
        cball.apply_palette(self.colorfb, self.colormap, self.palette)
        cball.array_interval_multiply(fbuf, fbuf, self.colorfb)


