
import uarray
import cball
from ani.gradient import get_smooth_wave

class Faces:

    def __init__(self, leds, config=None):
        assert 'faces' in leds.circuits
        self.colors = [ (0xff,0xe6,0x9c), (0xff,0x99,23) ]
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

        self.phase = 0
        self.phase_max = 5*7*1000

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
            w = self.wave[ int(2048. * ( (self.phase*m)%self.phase_max / self.phase_max) ) ] + 255
            for i,v in enumerate(self.group):
                if v == a:
                    fbuf[i*3  ] = (self.colors[v][0]*w) >> 9
                    fbuf[i*3+1] = (self.colors[v][1]*w) >> 9
                    fbuf[i*3+2] = (self.colors[v][2]*w) >> 9

        self.phase = (self.phase+self.speed)%self.phase_max

