
import uarray
import cball

class Orbit:

    def __init__(self, leds):
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
            [  -1,-1,-1,     0,         0,       255*0.394,
               -1,-1,-1,   255*0.394, 255*0.394,  64*0.394,
               -1,-1,-1,   255*2.463,  32*2.463, 127*2.463   ]
        )

    def update(self, n):
        cball.orbit_update(self.objects, self.Gmdt2, n)
        for i in range(0, len(self.shader_flat), 6):
            self.shader_flat[i  ] = self.objects[i  ]
            self.shader_flat[i+1] = self.objects[i+1]
            self.shader_flat[i+2] = self.objects[i+2]

    def next_frame(self, fbuf):
        self.update(10)
        cball.bytearray_memset(fbuf, 0)
        cball.shader(fbuf, self.leds.flat_data, self.shader_flat)

