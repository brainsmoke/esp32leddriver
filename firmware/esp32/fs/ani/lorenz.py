
import uarray
import cball

class Lorenz:

    def __init__(self, leds, config=None):
        self.leds = leds
        self.attractors = uarray.array('f',
        #             position                  sigma, rho, beta
        [  -8.268262, -5.752763, 29.76239,       10., 28., 8./3,
           -10.43131, -8.260322, 31.83801,       10., 28., 8./3,
           -10.51103, -8.488544, 31.78154,       10., 28., 8./3,
           -8.401729, -5.768218, 30.03114,       10., 28., 8./3,
           -11.96497, -12.55917, 30.83265,       10., 28., 8./3,
           -9.486767, -6.589458, 31.48488,       10., 28., 8./3 ]
        )

        self.shader_flat = uarray.array('f',
        #      position            color * intensity
            [  -1,-1,-1,     0      ,   0      , 255*1.44,
               -1,-1,-1,   255*1.44 , 255*1.44 ,  64*1.44,
               -1,-1,-1,   255*1.44 ,  32*1.44 , 127*1.44,
               -1,-1,-1,     0*1.44 , 255*1.44 ,  25*1.44,
               -1,-1,-1,   255*1.44 ,  55*1.44 ,  34*1.44,
               -1,-1,-1,    55*1.44 ,  32*1.44 , 127*1.44    ]
        )

        if config:
            for i in range(6):
                config.add_color('color_'+chr(ord('a')+i),
                                 lambda    i=i: self.get_color(i),
                                 lambda c, i=i: self.set_color(i, c), caption='Color '+chr(ord('A')+i))
        self.set_speed(10)
        if config:
            config.add_slider('speed', 0, 20, .01, self.get_speed, self.set_speed, caption="speed")

    def set_speed(self, speed):
        self.speed_db = speed
        self.speed = 10**(speed/10) / 100000.

    def get_speed(self):
        return self.speed_db

    def get_color(self, ix):
        return tuple( int(c/1.44+.5) for c in self.shader_flat[ix*6+3:ix*6+6] )

    def set_color(self, ix, color):
        self.shader_flat[ix*6+3] = color[0] * 1.44
        self.shader_flat[ix*6+4] = color[1] * 1.44
        self.shader_flat[ix*6+5] = color[2] * 1.44

    def update(self, n):
        cball.lorenz_update(self.attractors, self.speed, n)
        # copy positions, rotate axes for nicer effects
        self.shader_flat[0] = -self.attractors[0]*.11
        self.shader_flat[1] =  self.attractors[1]*.11
        self.shader_flat[2] = -self.attractors[2]*.11

        self.shader_flat[6] = -self.attractors[6]*.11
        self.shader_flat[7] =  self.attractors[7]*.11
        self.shader_flat[8] = -self.attractors[8]*.11

        self.shader_flat[12] = -self.attractors[12]*.11
        self.shader_flat[13] =  self.attractors[13]*.11
        self.shader_flat[14] = -self.attractors[14]*.11

        self.shader_flat[18] =  self.attractors[18]*.11
        self.shader_flat[19] =  self.attractors[20]*.11
        self.shader_flat[20] = -self.attractors[19]*.11

        self.shader_flat[24] =  self.attractors[24]*.11
        self.shader_flat[25] =  self.attractors[26]*.11
        self.shader_flat[26] = -self.attractors[25]*.11

        self.shader_flat[30] =  self.attractors[30]*.11
        self.shader_flat[31] = -self.attractors[32]*.11
        self.shader_flat[32] =  self.attractors[31]*.11

    def next_frame(self, fbuf):
        self.update(10)
        cball.bytearray_memset(fbuf, 0)
        cball.shader(fbuf, self.leds.flat_data, self.shader_flat)

