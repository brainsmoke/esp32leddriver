
import uarray
import cball

_wave = None
def get_wave():
    global _wave
    if not _wave:
        _wave = uarray.array('H', 0 for _ in range(500) )
        bump = memoryview(_wave)
        cball.wave_for_gradient_lut(_wave)
    return _wave


class Rutherford:

    def __init__(self, leds, config=None, **kwargs):
        assert leds.n_leds == 240

        self.colors = [ cball.ColorDrift(200+i*30, 3) for i in range(6) ]
        
        self.fb = uarray.array('H', 0 for _ in range( leds.n_leds * 3 ) )
        self.colormap = bytearray( leds.n_leds )
        for i in range(len(self.colormap)):
            self.colormap[i] = i//40
        self.palette = uarray.array('H', 0 for _ in range( 6 * 3 ) )
        self.colorfb  = uarray.array('H', 0 for _ in range( leds.n_leds * 3 ) )
        self.phase = 0
        self.wave = get_wave()

        factors = (11,13,15,17,19,23)
        self.phase_max = 1
        for f in factors:
            self.phase_max *= f

        self.div = tuple( self.phase_max // x for x in factors )

        self.set_speed(9)
        if config:
            config.add_slider('speed', 9, 22, 1, self.get_speed, self.set_speed, caption="speed")

        self.set_fade(.96)
        if config:
            config.add_slider('fade', .9, .99, .005, self.get_fade, self.set_fade, caption="fade")

    def set_speed(self, speed):
        self.speed_db = speed
        self.speed = int(self.phase_max * 10**(speed/10) // 100000)

    def get_speed(self):
        return self.speed_db

    def set_fade(self, fade):
        self.fade = fade

    def get_fade(self):
        return self.fade

    @micropython.native
    def next_frame(self, out):
        fb = self.fb
        phase = self.phase
        div = self.div
        wave = self.wave

        cball.array_set(out, 0)
        cball.array_blend(fb, out, self.fb, self.fade)

        for c in range(6):
            self.colors[c].next_color_into(self.palette, c*3)
            p = phase % self.div[c] * 2000 // self.div[c]
            for i in range(20):
                p += 100
                if p > 2000:
                    p -= 2000
                if p < 500:
                    w = wave[p]
                    ix = c*120 + i*6
                    out[ix  ] = w
                    out[ix+1] = w
                    out[ix+2] = w
                    out[ix+3] = w
                    out[ix+4] = w
                    out[ix+5] = w

        cball.apply_palette(self.colorfb, self.colormap, self.palette)
        cball.array_interval_multiply(out, out, self.colorfb)
        cball.array_max(fb, fb, out)
        cball.array_copy(out, fb)

        self.phase += self.speed
        self.phase %= self.phase_max
