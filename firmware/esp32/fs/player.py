import utime, gc

import cball

class Off:
    def next_frame(self, fbuf):
        cball.bytearray_memset(fbuf, 0)

class Player:

    def __init__(self, driver, leds):
        self._ani = []
        self._cur = 0
        self._off = Off()
        self._cur_ani = self._off
        self._driver = driver

        self._fadeout = bytearray(leds.n_leds * 3)
        self._fb = bytearray(leds.n_leds * 3)
        self._fade = 60

        self._old_gamma = self._new_gamma = driver.get_gamma()
        self._old_brightness = self._new_brightness = driver.get_brightness()
        self._gamma_fade = 60

    def _set_animation(self, ani):
        self._cur_ani = ani
        self._fadeout, self._fb = self._fb, self._fadeout
        self._fade = 0

    def get_selected(self):
        if self._cur_ani != self._off:
            return self._ani[self._cur][0]
        else:
            return 'Off'

    def select(self, name):
        for i in range(len(self._ani)):
            if self._ani[i][0] == name:
                self._cur = i
                self._set_animation(self._ani[self._cur][1])

    def add_animation(self, name, ani):
        self._ani.append( (name, ani) )

    def next(self):
        if self._cur_ani != self._off:
            self._cur = (self._cur+1)%len(self._ani)
            self._set_animation(self._ani[self._cur][1])

    def previous(self):
        if self._cur_ani != self._off:
            self._cur = (self._cur-1)%len(self._ani)
            self._set_animation(self._ani[self._cur][1])

    def off(self):
        self._set_animation(self._off)

    def is_off(self):
        return self._cur_ani == self._off

    def on(self):
        if len(self._ani) > 0:
            self._set_animation(self._ani[self._cur][1])

    def is_on(self):
        return self._cur_ani != self._off

    def set_brightness(self, value):
        if 0 <= value <= 1:
            self._old_brightness = self._driver.get_brightness()
            self._old_gamma = self._driver.get_gamma()
            self._gamma_fade = 0
            self._new_brightness = value

    def get_brightness(self):
        return self._new_brightness

    def set_gamma(self, value):
        if 1 <= value <= 4:
            self._old_brightness = self._driver.get_brightness()
            self._old_gamma = self._driver.get_gamma()
            self._gamma_fade = 0
            self._new_gamma = value

    def get_gamma(self):
        return self._new_gamma

    def run(self):
        try:
            t_next = utime.ticks_add(utime.ticks_us(), 16666)
            while True:
                fb = self._fb
                self._cur_ani.next_frame(fb)
                if self._fade < 60:
                    self._fade += 1
                    cball.bytearray_blend(fb, self._fadeout, fb, self._fade/60.)

                if self._gamma_fade < 60:
                    self._gamma_fade += 1
                    cur_gamma = ( self._new_gamma *     self._gamma_fade +
                                  self._old_gamma * (60-self._gamma_fade) ) / 60
                    cur_brightness = ( self._new_brightness *     self._gamma_fade +
                                       self._old_brightness * (60-self._gamma_fade) ) / 60

                    self._driver.calc_gamma_map(gamma=cur_gamma, brightness=cur_brightness)

                dt = utime.ticks_diff(t_next, utime.ticks_us())
                if dt < -2000:
#                    print(dt)
                    t_next = utime.ticks_us()

                elif dt > 0:
#                    while utime.ticks_diff(utime.ticks_us(),t_next) < 0:
#                        pass
                    utime.sleep_us(dt)

                t_next = utime.ticks_add(t_next, 16666)
                self._driver.writefrom(fb)
        finally:
            self._off.next_frame(fb)
            self._driver.writefrom(fb)


