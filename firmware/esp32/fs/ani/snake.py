
import uarray, random
import cball
import model

MAGIC=b'\1RANDOMWALK'

class Snake:

    def __init__(self, leds, config=None, snake_len = 7, snake_count = 2, max_speed = 300, mirror_leds=False, **kwargs):

        f = model.open_file("randomwalk.bin", "rb")
        magic = f.read(11)
        assert magic == MAGIC
  
        d = f.read(5)
        n_choices = d[0]
        n_weight_rows = d[1] | (d[2]<<8)
        n_positions = d[3] | (d[4]<<8)

        self.mirror = mirror_leds
        assert n_positions*(1 + int(bool(mirror_leds))) == leds.n_leds

        self.weights = f.read(n_choices*n_weight_rows)
        assert len(self.weights) == n_choices*n_weight_rows

        self.moves = uarray.array('H', (0 for _ in range(n_choices*n_positions)))
        n_read = f.readinto(self.moves)
        assert n_read == 2*n_choices*n_positions

        self.turns = uarray.array('H', (0 for _ in range(n_choices*n_positions)))
        n_read = f.readinto(self.turns)
        assert n_read == 2*n_choices*n_positions

        f.close()

        assert snake_len * snake_count <= 65535

        self.occupied = uarray.array('H', (0 for _ in range( n_positions ) ) )
        self.snake_len = snake_len
        self.n_choices = n_choices
        self.pos = tuple( [0]*self.snake_len for _ in range(snake_count) )
        self.dir = [0] * snake_count
        self.occupied[0] = snake_count*self.snake_len

        self.color = [ cball.ColorDrift(128, 3, 42) for _ in range(snake_count) ]
        self.fb = uarray.array('H', (0 for _ in range( leds.n_leds * 3 ) ) )
        self.bfb = uarray.array('H', (0 for _ in range( leds.n_leds * 3 ) ) )
        self.phase = 0
        self.phase_max = 2000

        max_speed = min(self.phase_max, max(1, int(max_speed)))
        self.set_speed(max_speed//3)
        if config:
            config.add_slider('speed', 0, max_speed, 1, self.get_speed, self.set_speed, caption="speed")

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
                self.occupied[last_pos] -= 1
                if self.occupied[last_pos] == 0:
                    self.occupied[last_pos] = 0
                    if self.mirror:
                        ix = last_pos*6
                        fb[ix+3] = 0
                        fb[ix+4] = 0
                        fb[ix+5] = 0
                    else:
                        ix = last_pos*3

                    fb[ix  ] = 0
                    fb[ix+1] = 0
                    fb[ix+2] = 0
                for i in range(self.snake_len-1, 0, -1):
                    p[i] = p[i-1]
                self.random_step(x)
                new_pos = p[0]
                self.occupied[new_pos] += 1
                r,g,b = self.color[x].next_color()
                if self.mirror:
                    ix = new_pos*6
                    fb[ix+3] = r
                    fb[ix+4] = g
                    fb[ix+5] = b
                else:
                    ix = new_pos*3

                fb[ix  ] = r
                fb[ix+1] = g
                fb[ix+2] = b

        self.phase %= self.phase_max

