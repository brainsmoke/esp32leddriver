
import uarray, math, cmath
import cball

def clamp_byte(x):
    return min(255, max(0, int(x)))

def smooth_wave_point(theta):
    return clamp_byte( ((1-math.cos(theta*2*math.pi))/2)**2.6 * 255)

def get_wave(size=1024, wavesize=None):
    wave = bytearray(size)

    if wavesize == None:
        wavesize = size

    for i in range(min(size, wavesize)):
        wave[i] = smooth_wave_point(i/wavesize)
    return wave

class TestPatternA:

    def __init__(self, leds):
        self.edge_count = leds.n_leds//4//4
        self.led_ix = list(256*(i%self.edge_count) for i in range(self.edge_count))
        self.phase = 0
        self.phase_max = self.edge_count*256
        self.wave = get_wave(size=self.phase_max, wavesize=self.phase_max//6)
        print(self.wave)

    def next_frame(self, fbuf):
        cball.bytearray_memset(fbuf, 0)
        self.phase = (self.phase-1)%self.phase_max
        a, b, c, d = 0, self.edge_count*4*3, self.edge_count*4*3*2, self.edge_count*4*3*3
        for i in range(self.edge_count):
            val = self.wave[ (self.led_ix[i]+self.phase)%self.phase_max ]
            for j in range(4):
                edge_base = (i*4*3+j*3)
                fbuf[edge_base+a] = val
                fbuf[edge_base+a+1] = val//2

                fbuf[edge_base+b+2] = val
                fbuf[edge_base+b] = val//2

                fbuf[edge_base+c+1] = val
                fbuf[edge_base+c+2] = val//2

                fbuf[edge_base+d] = val
                fbuf[edge_base+d+2] = val//2

