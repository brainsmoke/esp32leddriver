
import uarray, cmath
import cball

from util import get_smooth_wave

class MultiWave:

    def __init__(self, leds, config=None, **kwargs):
        assert 'facets' in leds.circuits
        self.wave = get_smooth_wave()
        n = len(self.wave)

        facets = leds.circuits['facets']
        n_groups = max(len(f) for f in facets)

        self.colors = [ cball.ColorDrift(128*(i+3), 1) for i in range(n_groups) ]
        self.phases = uarray.array('H', (0 for _ in range(n_groups*3)))
        self.intensities = uarray.array('H', (0 for _ in range(n_groups*3)))

        self.groups = uarray.array('H', (0 for _ in range(leds.n_leds*3)))
        self.rotations = uarray.array('H', (0 for _ in range(leds.n_leds*3)))

        axes = ( (1, 2), (2, 0), (0, 1), (2, 1), (1, 0), (0, 2) )

        for f in facets:
            for i, l in enumerate(f):
                 self.groups[l*3  ] = i*3
                 self.groups[l*3+1] = i*3+1
                 self.groups[l*3+2] = i*3+2
                 pos = leds.positions[l]
                 a, b = axes[i%len(axes)]
                 signbits = i//6
                 a, b = pos[a], pos[b]
                 if signbits&1:
                     a=-a
                 if signbits&2:
                     b=-b
                 phase = int((n * (1+cmath.phase(complex(a,b)) / (cmath.pi*2))) % n)
                 self.rotations[l*3  ] = phase
                 self.rotations[l*3+1] = phase
                 self.rotations[l*3+2] = phase

        self.phase_inc = uarray.array('H', ( int(0x10000/((i+1)*2000/n_groups)) for i in range(n_groups)) )
        self.phase_pre = uarray.array('H', (0 for _ in range(n_groups)))

    @micropython.native
    def next_frame(self, out):
        phase_inc = self.phase_inc
        phase_pre = self.phase_pre
        phases = self.phases
        n = len(self.wave)

        for i in range(len(phase_pre)):
            phases[i*3+2] = phases[i*3+1] = phases[i*3] = (phase_pre[i]*n)>>16
            phase_pre[i] = (phase_pre[i] + phase_inc[i]) & 0xffff
            self.colors[i].next_color_into(self.intensities, i*3)

        cball.grouped_gradient(out, self.rotations, self.groups, self.wave, self.intensities, phases)

