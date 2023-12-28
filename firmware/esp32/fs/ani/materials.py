
import uarray
import cball

class Checkers:

    def __init__(self, leds, tmpfloat, config=None, **kwargs):
        # hacky determination of it being the right sphere
        assert leds.n_leds == 240
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
            [  -1,-1,-1,   0.394, 0.394, 0.394,
               -1,-1,-1,   0.394, 0.394, 0.394,
               -1,-1,-1,   1.25,  1.25,  1.25   ]
        )

        self.magnitude = [ .394/255, .394/255, 1.25/255 ]

        self.colors = [ (0xff,0xe5,0x99), (0xff,0x99,23) ]
        self.checkers = uarray.array('H', 0 for i in range(leds.n_leds * 3) )

        self.group = bytearray(leds.n_leds)
        for c in leds.circuits['faces']:
            for i in c:
                self.group[i] = int(len(c)==6)

        if config:
            for i in range(2):
                config.add_color('color_'+chr(ord('a')+i),
                                 lambda    i=i: self.get_color(i),
                                 lambda c, i=i: self.set_color(i, c), caption='Color '+chr(ord('A')+i))
            for i in range(3):
                config.add_color('light_'+chr(ord('a')+i),
                                 lambda    i=i: self.get_light(i),
                                 lambda c, i=i: self.set_light(i, c), caption='Light '+chr(ord('A')+i))

        self.update_colors()

        self.set_speed(10)
        if config:
            config.add_slider('speed', 0, 30, 1, self.get_speed, self.set_speed, caption="speed")

        self.tmpfloat = tmpfloat

    def set_speed(self, speed):
        self.speed = speed

    def get_speed(self):
        return self.speed

    def get_color(self, ix):
        return self.colors[ix]

    def set_color(self, ix, color):
        self.colors[ix]=color
        self.update_colors()

    def update_colors(self):
        for i in range(len(self.group)):
            self.checkers[i*3  ] = self.colors[self.group[i]][0]*0x101
            self.checkers[i*3+1] = self.colors[self.group[i]][1]*0x101
            self.checkers[i*3+2] = self.colors[self.group[i]][2]*0x101

    def get_light(self, ix):
        mag = self.magnitude[ix]
        return tuple( int(c/mag+.5) for c in self.shader_flat[ix*6+3:ix*6+6] )

    def set_light(self, ix, light):
        mag = self.magnitude[ix]
        self.shader_flat[ix*6+3] = light[0] * mag
        self.shader_flat[ix*6+4] = light[1] * mag
        self.shader_flat[ix*6+5] = light[2] * mag


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
        cball.array_interval_multiply(fbuf, fbuf, self.checkers)

class AlienPlanet:

    def __init__(self, leds, tmpfloat=None, config=None, **kwargs):
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
            [  -1,-1,-1,   0.394, 0.394,  .25*0.394,
               -1,-1,-1,   .25*0.394, 0.394, 0.394,
               -1,-1,-1,   1.25,  0.3125,  1.25   ]
        )

        self.magnitude = [ .394/255, .394/255, 1.25/255 ]

        from ani.gradient import Spiral
        self.spiral = Spiral(leds, None)
        self.spiral.set_speed(25)
        #self.spiral = Spiral(leds, config)
        self.fb = uarray.array('H', 0 for i in range(leds.n_leds*3))

        if config:
            for i in range(3):
                config.add_color('light_'+chr(ord('a')+i),
                                 lambda    i=i: self.get_light(i),
                                 lambda c, i=i: self.set_light(i, c), caption='Light '+chr(ord('A')+i))

        self.set_speed(10)
        if config:
            config.add_slider('speed', 0, 30, 1, self.get_speed, self.set_speed, caption="speed")

        if tmpfloat == None:
             tmpfloat = uarray.array('f', 0 for i in range(len(leds)*3) )

        self.tmpfloat = tmpfloat

    def set_speed(self, speed):
        self.speed = speed

    def get_speed(self):
        return self.speed


    def get_light(self, ix):
        mag = self.magnitude[ix]
        return tuple( int(c/mag+.5) for c in self.shader_flat[ix*6+3:ix*6+6] )

    def set_light(self, ix, light):
        mag = self.magnitude[ix]
        self.shader_flat[ix*6+3] = light[0] * mag
        self.shader_flat[ix*6+4] = light[1] * mag
        self.shader_flat[ix*6+5] = light[2] * mag

    def update(self, n):
        cball.orbit_update(self.objects, self.Gmdt2, n)
        for i in range(0, len(self.shader_flat), 6):
            self.shader_flat[i  ] = self.objects[i  ]
            self.shader_flat[i+1] = self.objects[i+1]
            self.shader_flat[i+2] = self.objects[i+2]

    def next_frame(self, fbuf):
        cball.array_set(fbuf, 65535)
        self.spiral.next_frame(self.fb)
        cball.array_blend(self.fb, self.fb, fbuf, .5)

        self.update(self.speed)
        cball.shader(self.tmpfloat, self.leds.flat_data, self.shader_flat)
        cball.framebuffer_floatto16(fbuf, self.tmpfloat)
        cball.array_interval_multiply(fbuf, fbuf, self.fb)

