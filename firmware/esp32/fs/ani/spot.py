
import uarray
import cball
import cmath
from math import pi, sqrt

def norm(v, amp=1):
    d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2] )
    if d > 0:
        f = amp/d
    else:
        f = 1.

    return (v[0]*f, v[1]*f, v[2]*f)

# not doing fancy quaternion math yet, for the curent input fields this will do
def get_chroma_vectors( v ):
    amp = v[2]
    chroma = v[4]
    xrot = cmath.rect(1, -v[0])
    yrot = cmath.rect(1, v[1])
    brightness = v[3]
    r, g, b = v[5], v[6], v[7]

    xz_scale, y = xrot.real, -xrot.imag
    x, z = xz_scale * yrot.imag, xz_scale * yrot.real
    dx = (   yrot.real * chroma,        0,            -yrot.imag * chroma)
    dy = (y*-yrot.imag * chroma, xz_scale * chroma, y*-yrot.real * chroma)

# all three components at a 'perfect' triangular disctance
#    vec_red = norm( (x + dx[0]* 0.766 + dy[0]*-0.643, 
#                     y + dx[1]* 0.766 + dy[1]*-0.643, 
#                     z + dx[2]* 0.766 + dy[2]*-0.643), amp=amp)

#    vec_green = norm( (x + dx[0]*-0.939 + dy[0]*-0.342, 
#                       y + dx[1]*-0.939 + dy[1]*-0.342, 
#                       z + dx[2]*-0.939 + dy[2]*-0.342), amp=amp)

#    vec_blue = norm( (x + dx[0]* 0.174 + dy[0]* 0.985, 
#                      y + dx[1]* 0.174 + dy[1]* 0.985, 
#                      z + dx[2]* 0.174 + dy[2]* 0.985), amp=amp)


# nicer look using trial and error
    vec_red = norm( (x + dx[0]* 0.266 + dy[0]*-1, 
                     y + dx[1]* 0.266 + dy[1]*-1, 
                     z + dx[2]* 0.266 + dy[2]*-1), amp=amp)

    vec_green = norm( (x + dx[0]*-0.339, 
                       y + dx[1]*-0.339, 
                       z + dx[2]*-0.339), amp=amp)

    vec_blue = norm( (x + dy[0]*1, 
                      y + dy[1]*1, 
                      z + dy[2]*1), amp=amp)

    color = (brightness*r, brightness*g, brightness*b)

    return vec_red, vec_green, vec_blue, color

def interpolate_ugly(vec_a, vec_b, d):

    vec_res = list(vec_a)

    # radians, interpolate the right way around
    y_axis_rotation_a = vec_a[1]
    y_axis_rotation_b = vec_b[1]

    if y_axis_rotation_b - y_axis_rotation_a > pi:
        vec_res[1] += 2*pi
    if y_axis_rotation_a - y_axis_rotation_b > pi:
        vec_res[1] -= 2*pi

    for i in range(len(vec_res)):
        vec_res[i] += d*(vec_b[i]-vec_res[i])

    return vec_res

class _Spot:
    def __init__(self, rot, amp, brightness, color, chroma=.5):
        # all variables in a list suitable for interpolation
        self.new = list(rot[:2]) + [amp, brightness, chroma] + list(color)
        self.old = list(rot[:2]) + [amp, brightness, chroma] + list(color)
        self.vectors = get_chroma_vectors(self.new)
        self.move = 60
        self.chroma = chroma

    def set_new(self, new):
        old = interpolate_ugly(self.old, self.new, self.move/60)
        vectors = get_chroma_vectors(new)
        self.old, self.new, self.vectors, self.move = old, new, vectors, 0

    def set_x_axis_rotation(self, r):
        new = list(self.new)
        new[0] = r
        self.set_new( new )

    def set_y_axis_rotation(self, r):
        new = list(self.new)
        new[1] = r
        self.set_new( new )

    def set_amp(self, amp):
        new = list(self.new)
        new[2] = amp
        self.set_new( new )

    def set_brightness(self, brightness):
        new = list(self.new)
        new[3] = brightness
        self.set_new( new )

    def set_chroma(self, chroma):
        new = list(self.new)
        new[4] = chroma
        self.set_new( new )

    def set_color(self, color):
        new = list(self.new)
        new[5] = color[0]
        new[6] = color[1]
        new[7] = color[2]
        self.set_new( new )

    def get_x_axis_rotation(self):
        return self.new[0]

    def get_y_axis_rotation(self):
        return self.new[1]

    def get_amp(self):
        return self.new[2]

    def get_brightness(self):
        return self.new[3]

    def get_chroma(self):
        return self.new[4]

    def get_color(self):
        return self.new[5:8]

    def get_vectors(self):
        if self.move < 60:
            self.move += 1
            return get_chroma_vectors(interpolate_ugly(self.old, self.new, self.move/60))
        else:
            return self.vectors

class Chroma:

    def __init__(self, leds, config=None):
        self.leds = leds
        self.spots = [ _Spot( (  0,    0 ), 2, 3, (255, 255, 255), .2 ) ]

        self.shader_flat = uarray.array('f',
        #      position            color * intensity
            [  -1,-1,-1,     0, 0, 0,
               -1,-1,-1,     0, 0, 0,
               -1,-1,-1,     0, 0, 0 ]

#               -1,-1,-1,     0, 0, 0,
#               -1,-1,-1,     0, 0, 0,
#               -1,-1,-1,     0, 0, 0 ]
        )

        config.add_color( 'color', self.spots[0].get_color, self.spots[0].set_color, caption="color" )
        config.add_slider('x_axis_rotation', -pi/2, pi/2, .01, self.spots[0].get_x_axis_rotation, self.spots[0].set_x_axis_rotation, caption='x-axis rotation')
        config.add_slider('y_axis_rotation', -pi,   pi,   .01, self.spots[0].get_y_axis_rotation, self.spots[0].set_y_axis_rotation, caption='y-axis rotation')
        config.add_slider('chroma', 0, 1, .01, self.spots[0].get_chroma, self.spots[0].set_chroma, caption='chroma')
        config.add_slider('brightness', 0, 12, .01, self.spots[0].get_brightness, self.spots[0].set_brightness, caption='intensity')

    def update(self):
        for i in range(1):
            r_pos,g_pos,b_pos,color = self.spots[i].get_vectors()

            r, g, b = color
            self.shader_flat[i*18+3]  = r
            self.shader_flat[i*18+10] = g
            self.shader_flat[i*18+17] = b

            x,y,z = r_pos
            self.shader_flat[i*18+0] = x
            self.shader_flat[i*18+1] = y
            self.shader_flat[i*18+2] = z

            x,y,z = g_pos
            self.shader_flat[i*18+6] = x
            self.shader_flat[i*18+7] = y
            self.shader_flat[i*18+8] = z

            x,y,z = b_pos
            self.shader_flat[i*18+12] = x
            self.shader_flat[i*18+13] = y
            self.shader_flat[i*18+14] = z

    def next_frame(self, fbuf):
        self.update()
        cball.bytearray_memset(fbuf, 0)
        cball.shader(fbuf, self.leds.flat_data, self.shader_flat)


class Spots:

    def __init__(self, leds, config=None):
        self.leds = leds
        self.spots = [ _Spot( (    0,  0 ), 2, 2, (255,   0,   0), .2 ) ,
                       _Spot( (-pi/2,  0 ), 2, 2, (  0, 255,   0), .2 ) ,
                       _Spot( (    0, pi ), 2, 2, (  0,   0, 255), .2 ) ]

        self.shader_flat = uarray.array('f',
        #      position            color * intensity
            [  -1,-1,-1,     0, 0, 0,
               -1,-1,-1,     0, 0, 0,
               -1,-1,-1,     0, 0, 0 ]
        )

        config.add_color( 'color_a', self.spots[0].get_color, self.spots[0].set_color, caption="spot 1" )
        config.add_slider('x_axis_rotation_a', -pi/2, pi/2, .01, self.spots[0].get_x_axis_rotation, self.spots[0].set_x_axis_rotation, caption='x-axis rotation')
        config.add_slider('y_axis_rotation_a', -pi,   pi,   .01, self.spots[0].get_y_axis_rotation, self.spots[0].set_y_axis_rotation, caption='y-axis rotation')
#        config.add_slider('amp_a', 0,   6,   .5, self.spots[0].get_amp, self.spots[0].set_amp, caption='amplitude')
#        config.add_slider('brightness', 0, 12, 1, self.spots[0].get_brightness, self.spots[0].set_brightness, caption='intensity')

        config.add_color( 'color_b', self.spots[1].get_color, self.spots[1].set_color, caption="spot 2" )
        config.add_slider('x_axis_rotation_b', -pi/2, pi/2, .01, self.spots[1].get_x_axis_rotation, self.spots[1].set_x_axis_rotation, caption='x-axis rotation')
        config.add_slider('y_axis_rotation_b', -pi,   pi,   .01, self.spots[1].get_y_axis_rotation, self.spots[1].set_y_axis_rotation, caption='y-axis rotation')

        config.add_color( 'color_c', self.spots[2].get_color, self.spots[2].set_color, caption="spot 3" )
        config.add_slider('x_axis_rotation_c', -pi/2, pi/2, .01, self.spots[2].get_x_axis_rotation, self.spots[2].set_x_axis_rotation, caption='x-axis rotation')
        config.add_slider('y_axis_rotation_c', -pi,   pi,   .01, self.spots[2].get_y_axis_rotation, self.spots[2].set_y_axis_rotation, caption='y-axis rotation')

    def update(self):
        for i in range(3):
            pos,_,_,color = self.spots[i].get_vectors()

            x,y,z = pos
            self.shader_flat[i*6+0] = x
            self.shader_flat[i*6+1] = y
            self.shader_flat[i*6+2] = z

            r, g, b = color
            self.shader_flat[i*6+3] = r
            self.shader_flat[i*6+4] = g
            self.shader_flat[i*6+5] = b

    def next_frame(self, fbuf):
        self.update()
        cball.bytearray_memset(fbuf, 0)
        cball.shader(fbuf, self.leds.flat_data, self.shader_flat)


