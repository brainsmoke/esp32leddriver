
import uarray, math, cmath
import cball

def clamp_u16(x):
    return min(65536, max(0, int(x)))

def smooth_wave_point(theta):
    return clamp_u16( ((1-math.cos(theta*2*math.pi))/2)**2.6 * 65535)

def get_wave(size=1024, wavesize=None):
    wave = uarray.array('H', 0 for _ in range(size))

    if wavesize == None:
        wavesize = size

    for i in range(min(size, wavesize)):
        wave[i] = smooth_wave_point(i/wavesize)
    return wave

class TestPatternA:

    def __init__(self, leds, **kwargs):
        self.edge_count = leds.n_leds//4//4
        self.led_ix = list(64*(i%self.edge_count) for i in range(self.edge_count))
        self.phase = 0
        self.phase_max = self.edge_count*64
        self.wave = get_wave(size=self.phase_max, wavesize=self.phase_max//6)
        print(self.wave)

    def next_frame(self, fbuf):
        cball.array_set(fbuf, 0)
        self.phase = (self.phase-1)%self.phase_max
        a, b, c, d = 0, self.edge_count*4*3, self.edge_count*4*3*2, self.edge_count*4*3*3
        for i in range(self.edge_count):
            val = self.wave[ (self.led_ix[i]+self.phase)%self.phase_max ]
            for j in range(4):
                edge_base = (i*4*3+j*3)
                fbuf[edge_base+a] = val//2//(j+1)
                fbuf[edge_base+a+1] = val//4//(j+1)

                fbuf[edge_base+b+2] = val//2//(j+1)
                fbuf[edge_base+b] = val//4//(j+1)

                fbuf[edge_base+c+1] = val//2//(j+1)
                fbuf[edge_base+c+2] = val//4//(j+1)

                fbuf[edge_base+d] = val//2//(j+1)
                fbuf[edge_base+d+2] = val//4//(j+1)

class TestPatternB:

    def __init__(self, leds, **kwargs):
        self.edge_count = leds.n_leds//4//4
        self.led_ix = list(256*(i%self.edge_count) for i in range(self.edge_count))
        self.phase = 0
        self.phase_max = self.edge_count*16
        self.wave = get_wave(size=self.phase_max)#, wavesize=self.phase_max//6)
        print(self.wave)

    def next_frame(self, fbuf):
        cball.array_set(fbuf, 0)
        self.phase = (self.phase-1)%self.phase_max
        a, b, c, d = 0, self.edge_count*4*3, self.edge_count*4*3*2, self.edge_count*4*3*3
        for i in range(self.edge_count):
            val = self.wave[ (self.led_ix[i]+self.phase)%self.phase_max ]
            for j in range(4):
                edge_base = (i*4*3+j*3)
                fbuf[edge_base+a] = val//2
                fbuf[edge_base+a+1] = val//4

                fbuf[edge_base+b+2] = val//2
                fbuf[edge_base+b] = val//4

                fbuf[edge_base+c+1] = val//2
                fbuf[edge_base+c+2] = val//4

                fbuf[edge_base+d] = val//2
                fbuf[edge_base+d+2] = val//4

class TestPatternC:

    def __init__(self, leds, **kwargs):
        self.circle_count = 20
        self.led_ix = list(16*(i%self.circle_count) for i in range(self.circle_count))
        self.phase = 0
        self.phase_max = self.circle_count*16
        self.wave = get_wave(size=self.phase_max)#, wavesize=self.phase_max//6)
        #self.remap = [0, 1, 3, 2, 6, 7, 5, 4, 8, 9, 11, 10, 14, 15, 13, 12, 16, 17, 19, 18, 198, 199, 197, 196, 192, 193, 195, 194, 190, 191, 189, 188, 184, 185, 187, 186, 182, 183, 181, 180, 62, 63, 61, 60, 64, 65, 67, 66, 70, 71, 69, 68, 72, 73, 75, 74, 78, 79, 77, 76, 136, 137, 139, 138, 134, 135, 133, 132, 128, 129, 131, 130, 126, 127, 125, 124, 120, 121, 123, 122, 82, 83, 81, 80, 84, 85, 87, 86, 102, 103, 101, 100, 24, 25, 27, 26, 22, 23, 21, 20, 156, 157, 159, 158, 150, 151, 149, 148, 220, 221, 223, 222, 218, 219, 217, 216, 228, 229, 231, 230, 142, 143, 141, 140, 144, 145, 147, 146, 162, 163, 161, 160, 204, 205, 207, 206, 202, 203, 201, 200, 96, 97, 99, 98, 90, 91, 89, 88, 40, 41, 43, 42, 38, 39, 37, 36, 48, 49, 51, 50, 224, 225, 227, 226, 174, 175, 173, 172, 176, 177, 179, 178, 30, 31, 29, 28, 32, 33, 35, 34, 94, 95, 93, 92, 104, 105, 107, 106, 110, 111, 109, 108, 232, 233, 235, 234, 238, 239, 237, 236, 44, 45, 47, 46, 114, 115, 113, 112, 116, 117, 119, 118, 210, 211, 209, 208, 212, 213, 215, 214, 154, 155, 153, 152, 164, 165, 167, 166, 170, 171, 169, 168, 52, 53, 55, 54, 58, 59, 57, 56]
        self.remap = list(range(240))

        print(self.wave)

    def next_frame(self, fbuf):
        r = self.remap
        cball.array_set(fbuf, 0)
        self.phase = (self.phase-1)%self.phase_max
        for i in range(20):
            val = self.wave[ (self.led_ix[i]+self.phase)%self.phase_max ]
            for j in range(6):
                ix = (j*20+i)*2
                fbuf[r[ix  ]*3 + 0] = val//2
                fbuf[r[ix+1]*3 + 2] = val//2

