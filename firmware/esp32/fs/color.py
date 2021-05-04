
from cball import HSItoRGB, wave_lut
from os import urandom

def get_wave_table(n):
    tab = bytearray(n)
    wave_lut(tab) # use C implementation for faster startp time
#    import math
#    for i in range(len(tab)):
#        theta = i/len(tab)
#        tab[i] = min(255, max(0, int(( (1-math.cos(theta*2*math.pi))/2) * 255)))
    return tab

def get_random_color():
    rand = urandom(4)
    h = ( rand[0]<<8 | rand[1] ) / 0xffff
    i = .5 + ( rand[2]<<8 | rand[3] ) / 0x1fffe
    s = 1.
    return HSItoRGB(h, s, i)

class ColorDrift():

    _wave_lut = get_wave_table(4096)

    def __init__(self, phase_max, n_colors, phase_initial=0):
        self.phase_dt = n_colors * 4096
        self.phase_shift = phase_max * 4096
        self.phase_max = self.phase_shift * n_colors
        self.phase_div = phase_max * n_colors
        self.phase = ( self.phase_dt * phase_initial ) % self.phase_max
        self.colors = [ bytes(3) ] * n_colors

    def next_color(self):

        r,g,b = 0,0,0
        p = self.phase
        for i in range(len(self.colors)):
            if p < self.phase_dt:
                self.colors[i] = get_random_color()
            wave = self._wave_lut[p//self.phase_div]
            r += self.colors[i][0] * wave
            g += self.colors[i][1] * wave
            b += self.colors[i][2] * wave
            p = ( p + self.phase_shift ) % self.phase_max

        self.phase += self.phase_dt
        self.phase %= self.phase_max

        return ( min(255, r>>8), min(255, g>>8), min(255, b>>8) )

if __name__ == '__main__':
    drift = ColorDrift(32, 3)
    for i in range(5000):
        print ( drift.next_color() )
