
import uarray,math
import cball
import util

class ShadowWalk:

    def __init__(self, leds, tmpfloat, config=None, **kwargs):
        self.leds = leds
        self.shader_flat = uarray.array('f',
        #      position            color * intensity
            [  -1,-1,-1,     0,         0,       0, ]
        )

        self.color = cball.ColorDrift(1024, 3)
        self.set_speed(6)
        self.set_brightness(.4)
        self.set_distance(.5)
        if config:
            config.add_slider('speed', 0, 25, 1, self.get_speed, self.set_speed, caption="speed")
            config.add_slider('brightness', 0, 1, .01, self.get_brightness, self.set_brightness, caption='intensity')
            config.add_slider('distance', .01, 1, .01, self.get_distance, self.set_distance, caption='distance')

        self.phase_x = 0
        self.phase_z = 0
        self.tmpfloat = tmpfloat
        self.mask = util.get_inside_mapping(leds)

    def set_speed(self, speed):
        self.speed = speed

    def get_speed(self):
        return self.speed

    def set_brightness(self, brightness):
        self.brightness = brightness

    def get_brightness(self):
        return self.brightness

    def set_distance(self, distance):
        self.distance = distance

    def get_distance(self):
        return self.distance

    def update(self, speed):
        theta = math.pi*2*self.phase_x/2345
        sinx, cosx = math.sin(theta), math.cos(theta)
        theta = math.pi*2*self.phase_z/24000
        sinz, cosz = math.sin(theta), math.cos(theta)
        radius=1-self.distance
        x, y, z = sinz*cosx, cosz*cosx, sinx
        r,g,b = self.color.next_color()
        br = self.brightness/0xffff
        self.shader_flat[0] = x*radius
        self.shader_flat[1] = y*radius
        self.shader_flat[2] = z*radius
        self.shader_flat[3] = r*br
        self.shader_flat[4] = g*br
        self.shader_flat[5] = b*br
        self.phase_x += speed
        self.phase_x %= 2345
        self.phase_z += speed
        self.phase_z %= 24000

    def next_frame(self, fbuf):
        self.update(self.speed)
        cball.shader(self.tmpfloat, self.leds.flat_data, self.shader_flat)
        cball.framebuffer_floatto16(fbuf, self.tmpfloat)
        if self.mask:
            cball.array_interval_multiply(fbuf, fbuf, self.mask)


