
import uarray, cmath
import cball, model

import util

class Gradient:

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

    def __init__(self, leds, config=None, map_file=None, **kwargs):

        self.wave = util.get_smooth_wave()
        f = len(self.wave)/65536

        json = model.load_json(map_file)
        self.rel_speed = float(json['speed'])
        self.rotations = uarray.array('H', (int(x*f) for x in json['rotations']) )
        del json

        self.phase_inc   = 0
        self.phase_pre   = 0
        self.phases      = uarray.array('H', (0,))
        self.intensities = uarray.array('H', (65535,))
        self.groups      = uarray.array('H', (0 for _ in range(len(self.rotations))) )

        assert len(self.groups) == leds.n_leds
        assert len(self.rotations) == leds.n_leds

        self.set_speed(25)

        if config:
            config.add_slider('speed', 1, 200, 1, self.get_speed, self.set_speed, caption="speed")


    def get_speed(self):
        return self.speed

    def set_speed(self, speed):
        self.speed = speed
        self.phase_inc = int(self.rel_speed*self.speed)

    @micropython.native
    def next_frame(self, out):
        self.phases[0] = (self.phase_pre*len(self.wave))>>16
        self.phase_pre += self.phase_inc
        cball.grouped_gradient(out, self.rotations, self.groups, self.wave, self.intensities, self.phases)

class WaveSelectGradient(Gradient):
    def __init__(self, leds, config=None, **kwargs):
        super().__init__(leds, config, **kwargs)
        if config:
            config.add_multiple_choice('wave', ('50%', '10%', 'sawtooth', 'smooth'), self.get_wave, self.set_wave, caption="waveform")

