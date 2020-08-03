
import uarray
import cball

class Lorenz:

    def __init__(self, leds):
        self.leds = leds
        self.attractors = uarray.array('f',
        #  position        sigma, rho, beta
        [  1,  2, .5,       10., 28., 8./3,
           1, .5,  1,       10., 28., 8./3,
          .5,  1,  1,       10., 28., 8./3,
           1,  2, .6,       10., 28., 8./3,
           1, .6,  2,       10., 28., 8./3,
          .6,  2,  1,       10., 28., 8./3 ]
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

    def update(self, n):
        cball.lorenz_update_flat(self.attractors, 1/5000., n)
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
        cball.shader_flat(fbuf, self.leds.flat_data, self.shader_flat)

