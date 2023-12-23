
import uarray, math, cmath
import cball

#def clamp_byte(x):
#    return min(255, max(0, int(x)))
#
#def smooth_wave_point(theta):
#    return clamp_byte( ((1-math.cos(theta*2*math.pi))/2)**2 * 255)

smooth_wave = None
def get_smooth_wave():
    global smooth_wave
    if not smooth_wave:
        smooth_wave = uarray.array('H', 0 for _ in range(2048))
        cball.wave_for_gradient_lut(smooth_wave) # same calculation, speed up boot time
#        for i in range(1025):
#            smooth_wave[i] = smooth_wave[-i] = smooth_wave_point( i/2048 )
    return smooth_wave

class BaseGradient:
    def next_frame(self, fbuf):
        self.phase = (self.phase+self.speed)%self.phase_max
        phi = int(2048. * self.phase / self.phase_max)
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, -phi*7, -phi*8, -phi*9)

    def get_speed(self):
        return self.speed

    def set_speed(self, speed):
        self.speed = speed

class Gradient(BaseGradient):

    def __init__(self, leds, config=None, **kwargs):
        n = 2048
        self.wave = get_smooth_wave()
        self.rotations = uarray.array('H', 0 for _ in range(leds.n_leds*3))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int((n * (1+cmath.phase(complex(y,z)) / (math.pi*2))) % n)
            self.rotations[i*3+1] = int((n * (1+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+2] = int((n * (1+cmath.phase(complex(x,y)) / (math.pi*2))) % n)

        self.phase = 0
        self.phase_max = 4*7*8*9* 10
        self.speed = 10
        if config:
            config.add_slider('speed', 0, 20, 1, self.get_speed, self.set_speed, caption="speed")

class Spiral(BaseGradient):

    def __init__(self, leds, config=None, **kwargs):
        n = 2048
        self.wave = get_smooth_wave()
        self.rotations = uarray.array('H', 0 for _ in range(leds.n_leds*3))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int((n * (1-x/2+cmath.phase(complex(y,z)) / (math.pi*2))) % n)
            self.rotations[i*3+1] = int((n * (1-y/2+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+2] = int((n * (1-z/2+cmath.phase(complex(x,y)) / (math.pi*2))) % n)

        self.phase = 0
        self.phase_max = 4*7*8*9* 10
        self.speed = 10
        if config:
            config.add_slider('speed', 0, 20, 1, self.get_speed, self.set_speed, caption="speed")

class Wobble:

    def __init__(self, leds, config=None, **kwargs):
        n = 2048
        self.wave = get_smooth_wave()
        self.rotations = uarray.array('H', 0 for _ in range(leds.n_leds*3))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int((n * (1+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+1] = int((n * (1+cmath.phase(complex(z,x)) / (math.pi*2))) % n)
            self.rotations[i*3+2] = int( (786*y) % n )

        self.phase = 0
        self.phase_max = 512*3*3* 10
        self.speed = 10
        if config:
            config.add_slider('speed', 0, 20, 1, self.get_speed, self.set_speed, caption="speed")

    def get_speed(self):
        return self.speed

    def set_speed(self, speed):
        self.speed = speed

    def next_frame(self, fbuf):
        self.phase = (self.phase+self.speed)%self.phase_max
        phi = self.phase / self.phase_max
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, int(phi*24576), int(phi*6144), 0)
        cball.wobble(fbuf, self.rotations, 2, phi)

class InsideWobble(Wobble):

    def __init__(self, leds, config=None, **kwargs):
        assert True in leds.inside
        self.mask = uarray.array('H', int(leds.inside[i//3])*0xffff0 for i in range(leds.n_leds*3))
        super().__init__(leds, config, **kwargs)

    def next_frame(self, fbuf):
        super().next_frame(fbuf)
        cball.array_interval_multiply(fbuf, fbuf, self.mask)

class ConfigMode:

    def __init__(self, leds, config=None, **kwargs):
        self.wave = get_smooth_wave()
        self.rotations = uarray.array('H', 0 for _ in range(leds.n_leds*3))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int( (768*y) % 2048 )
            self.rotations[i*3+1] = 0
            self.rotations[i*3+2] = 0

        self.phase = 0
        self.phase_max = 2048
        self.speed = 10

    def next_frame(self, fbuf):
        self.phase = (self.phase+self.speed)%self.phase_max
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, self.phase, 0, 0)

