
import uarray, math, cmath
import cball

def clamp_byte(x):
    return min(255, max(0, int(x)))

def smooth_wave_point(theta):
    return clamp_byte( ((1+math.sin(theta*2*math.pi))/2)**2 * 255)

smooth_wave = None
def get_smooth_wave():
    global smooth_wave
    if not smooth_wave:
        smooth_wave = bytes( smooth_wave_point(i/1024) for i in range(1024) )
    return smooth_wave

class BaseGradient:
    def next_frame(self, fbuf):
        self.phase = (self.phase+1)%self.phase_max
        phi = int(1024. * self.phase / self.phase_max)
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, -phi*7, -phi*8, -phi*9)

class Gradient(BaseGradient):

    def __init__(self, leds):
        n = 1024
        self.wave = get_smooth_wave()
        self.rotations = uarray.array('H', 0 for _ in range(leds.n_leds*3))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int((n * (1+cmath.phase(complex(y,z)) / (math.pi*2))) % n)
            self.rotations[i*3+1] = int((n * (1+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+2] = int((n * (1+cmath.phase(complex(x,y)) / (math.pi*2))) % n)

        self.phase = 0
        self.phase_max = 4*7*8*9

class Spiral(BaseGradient):

    def __init__(self, leds):
        n = 1024
        self.wave = get_smooth_wave()
        self.rotations = uarray.array('H', 0 for _ in range(leds.n_leds*3))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int((n * (1-x/2+cmath.phase(complex(y,z)) / (math.pi*2))) % n)
            self.rotations[i*3+1] = int((n * (1-y/2+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+2] = int((n * (1-z/2+cmath.phase(complex(x,y)) / (math.pi*2))) % n)

        self.phase = 0
        self.phase_max = 4*7*8*9

class Wobble:

    def __init__(self, leds):
        n = 1024
        self.wave = get_smooth_wave()
        self.rotations = uarray.array('H', 0 for _ in range(leds.n_leds*3))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int((n * (1+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+1] = int((n * (1+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+2] = int( (786*y) % n )

        self.phase = 0
        self.phase_max = 512*3*3

    def next_frame(self, fbuf):
        self.phase = (self.phase+1)%self.phase_max
        phi = self.phase / self.phase_max
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, int(phi*12288), int(phi*3072), 0)
        cball.wobble(fbuf, self.rotations, 2, phi)

