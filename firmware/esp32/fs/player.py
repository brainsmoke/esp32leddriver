import utime, uarray, gc, math

import cball

class Off:
    def next_frame(self, fbuf):
        cball.array_set(fbuf, 0)

class Player:

    def __init__(self, driver, leds):
        import _thread
        self._ani = []
        self._cur = 0
        self._off = Off()
        self._cur_ani = self._off
        self._driver = driver
        self._lock = _thread.allocate_lock()

        self._fade_fb = uarray.array('H', 0 for _ in range(leds.n_leds * 3))
        self._fb = uarray.array('H', 0 for _ in range(leds.n_leds * 3))
        self._fade = 60

        self._old_gamma = self._cur_gamma = self._new_gamma = driver.get_gamma()
        self._old_brightness = self._cur_brightness = self._new_brightness = math.sqrt(driver.get_brightness())
        self._gamma_fade = 60

    def _set_animation(self, index):
        self._lock.acquire()
        if index == None or len(self._ani) == 0:
            self._cur_ani = self._off
        else:
            self._cur = index
            self._cur_ani = self._ani[self._cur][1]

        self._fade_fb, self._fb = self._fb, self._fade_fb
        self._fade = 0
        self._lock.release()

    def get_selected(self):
        if self.is_off():
            return 'Off'
        else:
            return self._ani[self._cur][0]

    def select(self, name):
        for i in range(len(self._ani)):
            if self._ani[i][0] == name:
                self._set_animation(i)
                break

    def add_animation(self, name, ani):
        self._ani.append( (name, ani) )

    def next(self):
        if self.is_on():
            self._set_animation( (self._cur+1)%len(self._ani) )

    def previous(self):
        if self.is_on():
            self._set_animation( (self._cur-1)%len(self._ani) )

    def off(self):
        self._set_animation(None)

    def is_off(self):
        return self._cur_ani == self._off

    def on(self):
        if len(self._ani) > 0:
            self._set_animation( self._cur )

    def is_on(self):
        return self._cur_ani != self._off

    def set_brightness(self, value):
        if 0 <= value <= 1:
            self._lock.acquire()
            self._old_brightness = self._cur_brightness
            self._old_gamma = self._cur_gamma
            self._gamma_fade = 0
            self._new_brightness = value
            self._lock.release()

    def get_brightness(self):
        return self._new_brightness

    def set_gamma(self, value):
        if 1 <= value <= 4:
            self._lock.acquire()
            self._old_brightness = self._cur_brightness
            self._old_gamma = self._cur_gamma
            self._gamma_fade = 0
            self._new_gamma = value
            self._lock.release()

    def get_gamma(self):
        return self._new_gamma

    def run(self):
        try:
#            t_next = utime.ticks_add(utime.ticks_us(), 16666)
            while True:
                self._lock.acquire()
                ani        = self._cur_ani
                fb         = self._fb
                fade_fb    = self._fade_fb
                fade       = self._fade
                self._lock.release()
                try:
#                    t_0 = utime.ticks_us();
                    ani.next_frame(fb)
#                    t_1 = utime.ticks_us();
#                    print( t_1-t_0 )
                except KeyboardInterrupt as err:
                    raise err
                except Exception as err:
                    print(err)
                    print ("animation failed, removing: {}".format(self._cur_ani.__class__.__name__))
                    self._ani.pop(self._cur)
                    if len(self._ani) == 0:
                        break

                    self._set_animation( self._cur%len(self._ani) )

                if self._fade < 60:
                    self._fade += 1
                    cball.array_blend(fb, fade_fb, fb, fade/60.)

                if self._gamma_fade < 60:
                    self._lock.acquire()
                    self._gamma_fade += 1
                    self._cur_gamma = ( self._new_gamma *     self._gamma_fade +
                                        self._old_gamma * (60-self._gamma_fade) ) / 60
                    self._cur_brightness = ( self._new_brightness *     self._gamma_fade +
                                             self._old_brightness * (60-self._gamma_fade) ) / 60

                    self._lock.release()
                    self._driver.calc_gamma_map(gamma=self._cur_gamma, brightness=self._cur_brightness**2)

#                dt = utime.ticks_diff(t_next, utime.ticks_us())
#                if dt < -2000:
#                    print(dt)
#                    t_next = utime.ticks_us()

#                elif dt > 0:
#                    utime.sleep_us(dt)

#                t_next = utime.ticks_add(t_next, 16666)
                self._driver.writefrom16(fb)
        finally:
            self._off.next_frame(fb)
            self._driver.writefrom16(fb)

