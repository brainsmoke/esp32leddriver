
import uarray, cmath
import cball

from util import get_smooth_wave

class MultiWave:

    def __init__(self, leds, config=None, **kwargs):
        assert 'leds' in leds.groups
        self.wave = get_smooth_wave()
        n = len(self.wave)

        groups = leds.groups['leds']
        n_groups = max(groups)+1

        self.colors = [ cball.ColorDrift(128*(i+3), 1) for i in range(n_groups) ]
        self.phases = uarray.array('H', (0 for _ in range(n_groups*3)))
        self.intensities = uarray.array('H', (0 for _ in range(n_groups*3)))

        self.groups = uarray.array('H', (0 for _ in range(leds.n_leds*3)))
        self.rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))

        axes = ( (1, 2), (2, 0), (0, 1), (2, 1), (1, 0), (0, 2) )

        for led, group in enumerate(groups):
             self.groups[led*3  ] = group*3
             self.groups[led*3+1] = group*3+1
             self.groups[led*3+2] = group*3+2
             pos = leds.positions[led]
             a, b = axes[group%len(axes)]
             a, b = pos[a], pos[b]
             phase = int((n * (1+cmath.phase(complex(a,b)) / (cmath.pi*2))) % n)
             self.rotations[led*3  ] = phase
             self.rotations[led*3+1] = phase
             self.rotations[led*3+2] = phase

        self.phase_inc = uarray.array('H', (0 for _ in range(n_groups)))
        self.phase_pre = uarray.array('H', (0 for _ in range(n_groups)))
        self.set_speed(40)
        self.chroma = n//9

        if config:
            config.add_slider('speed', 6, 58, 1, self.get_speed, self.set_speed, caption="speed")
            config.add_slider('chroma', 0, n//3, 1, self.get_chroma, self.set_chroma, caption="chroma")

    def get_speed(self):
        return self.speed

    def set_speed(self, speed):
        self.speed = speed
        x = speed*len(self.phase_inc)
        for i in range(len(self.phase_inc)):
            self.phase_inc[i] = int(x/(i+1))


    def get_chroma(self):
        return self.chroma

    def set_chroma(self, chroma):
        self.chroma = chroma

    @micropython.native
    def next_frame(self, out):
        phase_inc = self.phase_inc
        phase_pre = self.phase_pre
        phases = self.phases
        n = len(self.wave)

        for i in range(len(phase_pre)):
            p = (phase_pre[i]*n)>>16
            phases[i*3  ] = (p+self.chroma)%n
            phases[i*3+1] =  p
            phases[i*3+2] = (p-self.chroma)%n
            phase_pre[i] = (phase_pre[i] + phase_inc[i]) & 0xffff
            self.colors[i].next_color_into(self.intensities, i*3)

        cball.grouped_gradient(out, self.rotations, self.groups, self.wave, self.intensities, phases)

