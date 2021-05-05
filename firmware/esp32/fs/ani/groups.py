
import uarray
import cball
from ani.gradient import get_smooth_wave

class Faces:

    def __init__(self, leds, config=None):
        assert 'faces' in leds.circuits
        self.colors = [ (0x20,0x18,0x5), (0xff,0x99,23) ]
        self.group = bytearray(leds.n_leds)
        self.wave = get_smooth_wave()

        for c in leds.circuits['faces']:
            for i in c:
                self.group[i] = int(len(c)==6)


        if config:
            for i in range(2):
                config.add_color('color_'+chr(ord('a')+i),
                                 lambda    i=i: self.get_color(i),
                                 lambda c, i=i: self.set_color(i, c), caption='Color '+chr(ord('A')+i))

        self.set_speed(10)
        if config:
            config.add_slider('speed', 0, 20, .01, self.get_speed, self.set_speed, caption="speed")

        self.set_min_fade(128)
        if config:
            config.add_slider('min_fade', 0, 257, 1, self.get_min_fade, self.set_min_fade, caption="fade minimum")

        self.phase = 0
        self.phase_max = 5*7*1000

    def set_min_fade(self, min_fade):
        self.min_fade = min_fade

    def get_min_fade(self):
        return self.min_fade

    def set_speed(self, speed):
        self.speed_db = speed
        self.speed = int(10**(speed/10))

    def get_speed(self):
        return self.speed_db

    def get_color(self, ix):
        return self.colors[ix]

    def set_color(self, ix, color):
        self.colors[ix]=color

    def next_frame(self, fbuf):
        
        for a, m in ( (0,5), (1,7) ):
            w = self.wave[ int(2048. * ( (self.phase*m)%self.phase_max / self.phase_max) ) ]
            w = self.min_fade * 255 + (257-self.min_fade) * w
            r = (self.colors[a][0]*w)>>16
            g = (self.colors[a][1]*w)>>16
            b = (self.colors[a][2]*w)>>16
            for i in range(len(self.group)):
                if self.group[i] ==a:
                    fbuf[i*3  ] = r
                    fbuf[i*3+1] = g
                    fbuf[i*3+2] = b

        self.phase = (self.phase+self.speed)%self.phase_max

