
import uarray, math, cmath
import cball

import util

class BaseGradient:
    def next_frame(self, fbuf):
        self.phase = (self.phase_max+self.phase-self.speed)%self.phase_max
        phi = len(self.wave) * self.phase / self.phase_max
        phi_r, phi_g, phi_b = int(phi * 7), int(phi * 8), int(phi * 9)
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, phi_r, phi_g, phi_b)
        if self.mask:
            cball.array_interval_multiply(fbuf, fbuf, self.mask)

    def get_speed(self):
        return self.speed

    def set_speed(self, speed):
        self.speed = speed

    def get_mask(self):
        if self.mask == self.inside_mask:
            return 0
        elif self.mask == self.outside_mask:
            return 1
        else:
            return 2

    def set_mask(self, mask):
        if mask == 0:
            self.mask = self.inside_mask
        elif mask == 1:
            self.mask = self.outside_mask
        else:
            self.mask = None

    def set_wave(self, wave):
        if wave == 0:
            self.wave = util.get_half_duty_pwm()
        elif wave == 1:
            self.wave = util.get_small_duty_pwm()
        elif wave == 2:
            self.wave = util.get_sawtooth_wave()
        else:
            self.wave = util.get_smooth_wave()

    def get_wave(self):
        if self.wave == util.get_half_duty_pwm():
            return 0
        elif self.wave == util.get_small_duty_pwm():
            return 1
        elif self.wave == util.get_sawtooth_wave():
            return 2
        else:
            return 3

class Gradient(BaseGradient):

    def __init__(self, leds, config=None, **kwargs):
        self.wave = util.get_smooth_wave()
        self.mask = None
        n = len(self.wave)
        self.rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))

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

        if config and util.has_sides(leds):
            self.inside_mask = util.get_inside_mapping(leds)
            self.outside_mask = util.get_outside_mapping(leds)
            self.mask = self.outside_mask
            config.add_multiple_choice('mask', ['inside', 'outside', 'all'], self.get_mask, self.set_mask, caption="illuminate")

class WaveSelectGradient(Gradient):
    def __init__(self, leds, config=None, **kwargs):
        super().__init__(leds, config, **kwargs)
        if config:
            config.add_multiple_choice('wave', ('50%', '10%', 'sawtooth', 'smooth'), self.get_wave, self.set_wave, caption="waveform")

class Spiral(BaseGradient):

    def __init__(self, leds, config=None, **kwargs):
        self.wave = util.get_smooth_wave()
        self.mask = None
        n = len(self.wave)
        self.rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))

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

        if config and util.has_sides(leds):
            self.inside_mask = util.get_inside_mapping(leds)
            self.outside_mask = util.get_outside_mapping(leds)
            self.mask = self.outside_mask
            config.add_multiple_choice('mask', ['inside', 'outside', 'all'], self.get_mask, self.set_mask, caption="illuminate")

class Wobble(BaseGradient):

    def __init__(self, leds, config=None, **kwargs):
        self.wave = util.get_smooth_wave()
        self.mask = None
        n = len(self.wave)
        self.rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))

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

        if config and util.has_sides(leds):
            self.inside_mask = util.get_inside_mapping(leds)
            self.outside_mask = util.get_outside_mapping(leds)
            self.mask = self.outside_mask
            config.add_multiple_choice('mask', ['inside', 'outside', 'all'], self.get_mask, self.set_mask, caption="illuminate")

    def next_frame(self, fbuf):
        self.phase = (self.phase+self.speed)%self.phase_max
        phi = self.phase / self.phase_max
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, int(phi*49152), int(phi*12288), 0)
        cball.wobble(fbuf, self.rotations, 2, phi)
        if self.mask:
            cball.array_interval_multiply(fbuf, fbuf, self.mask)

class ConfigMode:

    def __init__(self, leds, config=None, **kwargs):
        self.wave = util.get_smooth_wave()
        n = len(self.wave)
        self.rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))

        for i in range(leds.n_leds):
            x,y,z = leds.positions[i]
            self.rotations[i*3  ] = int( (768*y) % n )
            self.rotations[i*3+1] = 0
            self.rotations[i*3+2] = 0

        self.phase = 0
        self.phase_max = 2048
        self.speed = 10

    def next_frame(self, fbuf):
        self.phase = (self.phase+self.speed)%self.phase_max
        cball.gradient(fbuf, self.rotations, self.wave, self.wave, self.wave, self.phase, 0, 0)

