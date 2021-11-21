
import uarray
import cball

class Orbit:

    def __init__(self, leds, tmpfloat, config=None, **kwargs):
        self.leds = leds
        self.Gmdt2 = -6.674e-11*5.97219e24*1.*1./(6.371e6**3)
        self.objects = uarray.array('f',
        #         position                                   velocity
        [  1.047088, 0.,       0.,              0.,          3.943998e-4, 1.274987e-3,
           0.,       0.,       1.470884,        1.134319e-3, 3.149046e-4, 0.,
           0.,       2.569612, 0.,             -4.989389e-4, 0.,          5.923611e-4]
        )

        self.shader_flat = uarray.array('f',
        #      position            color * intensity
            [  -1,-1,-1,     0,         0,       0.394,
               -1,-1,-1,   0.394, 0.394,  64/255*0.394,
               -1,-1,-1,   2.463,  32/255*2.463, 127/255*2.463   ]
        )

        self.magnitude = [ .394/255, .394/255, 2.463/255 ]

        if config:
            for i in range(3):
                config.add_color('color_'+chr(ord('a')+i),
                                 lambda    i=i: self.get_color(i),
                                 lambda c, i=i: self.set_color(i, c), caption='Color '+chr(ord('A')+i))

        self.set_speed(10)
        if config:
            config.add_slider('speed', 0, 30, 1, self.get_speed, self.set_speed, caption="speed")

        self.tmpfloat = tmpfloat

    def set_speed(self, speed):
        self.speed = speed

    def get_speed(self):
        return self.speed

    def get_color(self, ix):
        mag = self.magnitude[ix]
        return tuple( int(c/mag+.5) for c in self.shader_flat[ix*6+3:ix*6+6] )

    def set_color(self, ix, color):
        mag = self.magnitude[ix]
        self.shader_flat[ix*6+3] = color[0] * mag
        self.shader_flat[ix*6+4] = color[1] * mag
        self.shader_flat[ix*6+5] = color[2] * mag


    def update(self, n):
        cball.orbit_update(self.objects, self.Gmdt2, n)
        for i in range(0, len(self.shader_flat), 6):
            self.shader_flat[i  ] = self.objects[i  ]
            self.shader_flat[i+1] = self.objects[i+1]
            self.shader_flat[i+2] = self.objects[i+2]

    def next_frame(self, fbuf):
        self.update(self.speed)
        cball.shader(self.tmpfloat, self.leds.flat_data, self.shader_flat)
        cball.framebuffer_floatto16(fbuf, self.tmpfloat)

