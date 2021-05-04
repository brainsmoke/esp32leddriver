
import uarray
import cball
import color

_wave = None
def get_wave():
    global _wave
    if not _wave:
        _wave = bytearray(512)
        bump = memoryview(_wave)
        cball.wave_for_gradient_lut(_wave)
    return _wave


class Rutherford:

    def __init__(self, leds, config=None):
        assert leds.n_leds == 240
        

        self.colors = [ color.ColorDrift(200+i*30, 3) for i in range(6) ]
        
        self.fb = bytearray( leds.n_leds * 3 )
        self.phase = 0
        self.wave = get_wave()

        factors = (11,13,15,17,19,23)
        self.phase_max = 1
        for f in factors:
            self.phase_max *= f

        self.div = tuple( self.phase_max // x for x in factors )

        self.set_speed(15)
        if config:
            config.add_slider('speed', 10, 25, 1, self.get_speed, self.set_speed, caption="speed")

        self.set_fade(.95)
        if config:
            config.add_slider('fade', .9, .98, .005, self.get_fade, self.set_fade, caption="fade")

    def set_speed(self, speed):
        self.speed_db = speed
        self.speed = int(self.phase_max * 10**(speed/10) // 100000)

    def get_speed(self):
        return self.speed_db

    def set_fade(self, fade):
        self.fade = fade

    def get_fade(self):
        return self.fade

    def update(self, n):
        cball.orbit_update(self.objects, self.Gmdt2, n)
        for i in range(0, len(self.shader_flat), 6):
            self.shader_flat[i  ] = self.objects[i  ]
            self.shader_flat[i+1] = self.objects[i+1]
            self.shader_flat[i+2] = self.objects[i+2]

    def next_frame(self, fbuf):
        cball.bytearray_memset(fbuf, 0)
        cball.bytearray_blend(self.fb, fbuf, self.fb, self.fade)
        for ci, color in enumerate(self.colors):
            r, g, b = color.next_color()
            p = int(2000 * (self.phase % self.div[ci]) )//self.div[ci]
            for i in range(20):
                pi = (p+i*100) % 2000
                if pi < len(self.wave):
                    ix = ci*120 + 6*i
                    w = self.wave[pi]
                    wr = (w*r) >> 8 
                    wg = (w*g) >> 8 
                    wb = (w*b) >> 8 
                    self.fb[ix+0] = max(wr, self.fb[ix+0])
                    self.fb[ix+3] = max(wr, self.fb[ix+3])

                    self.fb[ix+1] = max(wg, self.fb[ix+1])
                    self.fb[ix+4] = max(wg, self.fb[ix+4])

                    self.fb[ix+2] = max(wb, self.fb[ix+2])
                    self.fb[ix+5] = max(wb, self.fb[ix+5])

        cball.bytearray_memcpy(fbuf, self.fb)

        self.phase += self.speed
        self.phase %= self.phase_max
