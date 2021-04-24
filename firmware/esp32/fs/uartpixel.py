
import machine, uarray

import cball

class UartPixel:

#    def set_gamma_orig(self, gamma, cutoff=0x18):
#        max_val = 0xff00
#        factor = max_val / (255.**gamma)
#        for i in range(256):
#            v = int(factor * i**gamma)
#            lo, hi = v&0xff, v>>8
#            if lo <= cutoff//2:
#                lo = 0
#            elif lo < cutoff:
#                lo = cutoff
#            elif lo > 256-cutoff//2:
#                lo, hi = 0, min(hi+1, max_val>>8)
#            elif lo > 256-cutoff:
#                lo = 256-cutoff
#            self.gamma_map[i] = lo | (hi<<8)

    def calc_gamma_map(self, gamma=None, brightness=None, cutoff=None):
        if gamma != None:
            if not 0.5 <= gamma <= 10.5:
                raise ValueError("brightness needs to be between 0.5 and 10.5")
            else:
                self.gamma = gamma

        if brightness != None:
            if not 0 <= brightness <= 1:
                raise ValueError("brightness needs to be between 0 and 1 inclusive")
            else:
                self.brightness = brightness

        if cutoff != None:
            if not 0 <= cutoff <= 127:
                raise ValueError("cutoff needs to be between 0 and 127 inclusive")
            else:
                self.cutoff = cutoff

        max_ = max(0, min(int(self.brightness * 0xff00), 0xff00))
        cball.calc_gamma_map_sieve( self.gamma_map, self.gamma, max_, self.cutoff )

    def set_gamma(self, gamma):
        self.calc_gamma_map(gamma=gamma)

    def get_gamma(self):
        return self.gamma

    def set_brightness(self, brightness):
        self.calc_gamma_map(brightness=brightness)

    def get_brightness(self):
        return self.brightness

    def set_cutoff(self, cutoff):
        self.calc_gamma_map(cutoff=cutoff)

    def get_cutoff(self, cutoff):
        return self.cutoff

    def __init__(self, baudrate, rx, tx, n, led_order="GRB", gamma=2.5, cutoff=0x18, framebuf=True):
        self.n = n
        if framebuf:
            self.buf = bytearray(n*3)
        self.outbuf = uarray.array('H', 0 for _ in range(n*3 + 2))
        self.led_order = led_order
        self.gamma_map = uarray.array('H', 0 for _ in range(256))
        self.calc_gamma_map(gamma=gamma, cutoff=cutoff, brightness=1.0)
        self.uart = machine.UART(1, baudrate=baudrate, rx=rx, tx=tx, txbuf=len(self.outbuf))

    def __setitem__(self, index, val):
        offset = index * 3
        self.buf[offset] = val[0]
        self.buf[offset+1] = val[1]
        self.buf[offset+2] = val[2]

    def __getitem__(self, index):
        offset = index * 3
        return ( self.buf[offset], self.buf[offset+1], self.buf[offset+2] )

    def fill(self, color):
        for i in range(self.n):
            self[i] = color

    def write(self):
        writefrom(self, self.buf)

    def writefrom(self, buf):
        cball.fillbuffer_gamma(self.outbuf, buf, self.led_order, self.gamma_map)
        self.uart.write(self.outbuf)
