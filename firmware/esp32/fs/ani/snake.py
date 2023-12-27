
import uarray, random
import cball
import model

MAGIC=b'\1RANDOMWALK'

class Snake:

    def __init__(self, leds, config=None, **kwargs):

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

        self.who = bytearray(leds.n_leds)
        self.max_snake_len = 7
        self.snake_len = self.max_snake_len
        self.n_choices = n_choices
        self.pos = ( [0]*self.max_snake_len, [0]*self.max_snake_len )
        self.dir = [0,0]
        self.who[0] = len(self.pos)*self.snake_len

        self.color = [ cball.ColorDrift(128, 3) for _ in range(2) ]
        self.fb = uarray.array('H', (0 for _ in range( leds.n_leds * 3 ) ) )
        self.bfb = uarray.array('H', (0 for _ in range( leds.n_leds * 3 ) ) )
        self.phase = 0
        self.phase_max = 2000

        self.set_speed(100)
        if config:
            config.add_slider('speed', 0, 200, 1, self.get_speed, self.set_speed, caption="speed")

    def set_speed(self, speed):
        self.speed = speed
        steps_per_frame = self.speed/self.phase_max
        self.blend = 1-0.75**steps_per_frame

    def get_speed(self):
        return self.speed

    def random_step(self, x):
        weights = self.weights[self.dir[x]*self.n_choices:(self.dir[x]+1)*self.n_choices]
        total_weight = sum(weights)
        c = random.randrange(0, total_weight)
        for i in range(len(weights)):
            if c < weights[i]:
                self.dir[x] = self.turns[self.pos[x][0]*self.n_choices+i]
                self.pos[x][0] = self.moves[self.pos[x][0]*self.n_choices+i]
                return
            c -= weights[i]
        assert False

    @micropython.native
    def next_frame(self, out):
        speed = self.speed
        self.phase += speed
        phase = self.phase
        brightness = min(self.phase_max, self.phase)

        bfb = self.bfb
        fb = self.fb
        cball.array_blend(bfb, bfb, fb, self.blend)
        cball.array_copy(out, bfb)

        snake_len = self.snake_len
        if self.phase >= self.phase_max:
            for x in range(len(self.pos)):
                p = self.pos[x]
                last_pos = p[snake_len-1]
                self.who[last_pos] -= 1
                if self.who[last_pos] == 0:
                    self.who[last_pos] = 0
                    fb[last_pos*3  ] = 0
                    fb[last_pos*3+1] = 0
                    fb[last_pos*3+2] = 0
                for i in range(self.snake_len-1, 0, -1):
                    p[i] = p[i-1]
                self.random_step(x)
                new_pos = p[0]
                self.who[new_pos] += 1
                r,g,b = self.color[x].next_color()
                fb[new_pos*3  ] = r
                fb[new_pos*3+1] = g
                fb[new_pos*3+2] = b

        self.phase %= self.phase_max

