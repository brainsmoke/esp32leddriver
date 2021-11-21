
import os, uarray
import cball

class Lorenz:

    def __init__(self, leds, tmpfloat, config=None, **kwargs):
        self.leds = leds

        random = os.urandom(6*2)
        a = [ ( (random[x*2] | random[x*2+1]<<8) - 32768 )/262144 for x in range(6) ]

        self.attractors = uarray.array('f',
        #             position                      sigma, rho, beta
        [  -8.268262, -5.752763+a[0], 29.76239,       10., 28., 8./3,
           -10.43131, -8.260322+a[1], 31.83801,       10., 28., 8./3,
           -10.51103, -8.488544+a[2], 31.78154,       10., 28., 8./3,
           -8.401729, -5.768218+a[3], 30.03114,       10., 28., 8./3,
           -11.96497, -12.55917+a[4], 30.83265,       10., 28., 8./3,
           -9.486767, -6.589458+a[5], 31.48488,       10., 28., 8./3 ]
        )

        self.shader_flat = uarray.array('f',
        #      position            color * intensity
            [  -1,-1,-1,     0,0,0,
               -1,-1,-1,     0,0,0,
               -1,-1,-1,     0,0,0,
               -1,-1,-1,     0,0,0,
               -1,-1,-1,     0,0,0,
               -1,-1,-1,     0,0,0   ]
        )

        self.colors = [
            (  55, 206,   0 ),
            ( 255, 255,  64 ),
            ( 255,  32, 127 ),
            (   0, 255, 255 ),
            ( 255,  55,  34 ),
            ( 255, 164,   0 ),
        ]
        self.mag = 1.44/257.
        for i in range(len(self.colors)):
            self.set_color(i, self.colors[i])

        if config:
            for i in range(6):
                config.add_color('color_'+chr(ord('a')+i),
                                 lambda    i=i: self.get_color(i),
                                 lambda c, i=i: self.set_color(i, c), caption='Color '+chr(ord('A')+i))
        self.set_speed(10)
        if config:
            config.add_slider('speed', 0, 20, .01, self.get_speed, self.set_speed, caption="speed")

        self.tmpfloat = tmpfloat

    def set_speed(self, speed):
        self.speed_db = speed
        self.speed = 10**(speed/10) / 100000.

    def get_speed(self):
        return self.speed_db

    def get_color(self, ix):
        return self.colors[ix]

    def set_color(self, ix, color):
        self.colors[ix] = color
        self.shader_flat[ix*6+3] = color[0] * self.mag
        self.shader_flat[ix*6+4] = color[1] * self.mag
        self.shader_flat[ix*6+5] = color[2] * self.mag

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
        cball.shader(self.tmpfloat, self.leds.flat_data, self.shader_flat)
        cball.framebuffer_floatto16(fbuf, self.tmpfloat)

