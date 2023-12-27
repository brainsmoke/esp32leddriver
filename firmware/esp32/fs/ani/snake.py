
import uarray, random
import cball
import model, util

MAGIC=b'\1RANDOMWALK'

_wave = None
def get_wave():
    global _wave
    if not _wave:
        _wave = uarray.array('H', 0 for _ in range(400) )
        bump = memoryview(_wave)
        cball.wave_for_gradient_lut(_wave)
    return _wave


class Snake:

    def __init__(self, leds, config=None, **kwargs):

        self.wave = util.get_smooth_wave()
        self.inside = leds.inside

        f = model.open_file("randomwalk.bin", "rb")
        magic = f.read(11)
        assert magic == MAGIC
  
        d = f.read(5)
        n_choices = d[0]
        n_weight_rows = d[1] | (d[2]<<8)
        n_positions = d[3] | (d[4]<<8)
        assert n_positions == leds.n_leds

        self.weights = f.read(n_choices*n_weight_rows)
        assert len(self.weights) == n_choices*n_weight_rows

        self.moves = uarray.array('H', (0 for _ in range(n_choices*n_positions)))
        n_read = f.readinto(self.moves)
        assert n_read == 2*n_choices*n_positions

        self.turns = uarray.array('H', (0 for _ in range(n_choices*n_positions)))
        n_read = f.readinto(self.turns)
        assert n_read == 2*n_choices*n_positions

        self.n_choices = n_choices
        self.pos = [0,0]
        self.dir = [0,0]

        self.color = [ cball.ColorDrift(512, 3) for _ in range(2) ]
        self.fb = uarray.array('H', (0 for _ in range( leds.n_leds * 3 ) ) )
        self.phase = 0
        self.phase_max = len(self.wave)//2

        self.fade = 1
        self.set_speed(100)
        if config:
            config.add_slider('speed', 0, 200, 1, self.get_speed, self.set_speed, caption="speed")
            config.add_slider('fade', .33, 3, .01, self.get_fade, self.set_fade, caption="fade")

    def set_speed(self, speed):
        self.speed = speed
        steps_per_frame = speed/self.phase_max/self.fade
        self.blend = 0.75**steps_per_frame

    def get_speed(self):
        return self.speed

    def set_fade(self, fade):
        self.fade = fade
        steps_per_frame = self.speed/self.phase_max/self.fade
        self.blend = 0.75**steps_per_frame

    def get_fade(self):
        return self.fade

    def random_step(self, x):
        weights = self.weights[self.dir[x]*self.n_choices:(self.dir[x]+1)*self.n_choices]
        total_weight = sum(weights)
        c = random.randrange(0, total_weight)
        for i in range(len(weights)):
            if c < weights[i]:
                self.dir[x] = self.turns[self.pos[x]*self.n_choices+i]
                self.pos[x] = self.moves[self.pos[x]*self.n_choices+i]
                return
            c -= weights[i]
        assert False

    @micropython.native
    def next_frame(self, out):
        speed = self.speed
        self.phase += speed
        phase = self.phase
        brightness = min(self.phase_max, self.phase)

        fb = self.fb
        cball.array_set(out, 0)
        cball.array_blend(fb, out, fb, self.blend)

        for x in range(len(self.pos)):
            p = self.pos[x]
            r,g,b = self.color[x].next_color()
            fb[p*3  ] = max(fb[p*3  ], (r*self.wave[phase])>>16)
            fb[p*3+1] = max(fb[p*3+1], (g*self.wave[phase])>>16)
            fb[p*3+2] = max(fb[p*3+2], (b*self.wave[phase])>>16)

        cball.array_copy(out, fb)

        if self.phase >= self.phase_max:
            for x in range(len(self.pos)):
                self.random_step(x)

        self.phase %= self.phase_max

