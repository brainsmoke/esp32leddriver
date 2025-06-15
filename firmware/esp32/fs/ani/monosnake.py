
import uarray, random
import cball

from . import snake

class Snake(snake.Snake):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        n_leds = len(self.fb)//3
        self.fb = uarray.array('H', (0 for _ in range( n_leds ) ) )
        self.bfb = uarray.array('H', (0 for _ in range( n_leds ) ) )

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
                        ix = last_pos*2
                        fb[ix+1] = 0
                    else:
                        ix = last_pos

                    fb[ix  ] = 0
                for i in range(self.snake_len-1, 0, -1):
                    p[i] = p[i-1]
                self.random_step(x)
                new_pos = p[0]
                self.occupied[new_pos] += 1
                if self.mirror:
                    ix = new_pos*2
                    fb[ix+1] = 65535
                else:
                    ix = new_pos

                fb[ix  ] = 65535

        self.phase %= self.phase_max

