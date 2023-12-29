
import machine, uarray

import cball
import _uartpixel

class UartPixel:

#    def set_gamma_orig(self, gamma, max_val=0xff00):
#        factor = max_val / (255.**gamma)
#        for i in range(256):
#            self.gamma_map[i] = max(0, min(max_val, int(factor * i**gamma)))

    def calc_gamma_map(self, gamma=None, brightness=None, cutoff=None):
        if gamma != None:
            if not 0.5 <= gamma <= 10.5:
                raise ValueError("gamma needs to be between 0.5 and 10.5")
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
        cball.calc_gamma_map( self.gamma_map, self.gamma, max_ )

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

    def create_framebuffer(self):
        return bytearray(self.n*3)

    def create_framebuffer16(self):
        return uarray.array('H', (0 for _ in range(self.n*3)))

    def __init__(self, baudrate, rx, tx, n, led_order="GRB", brightness=1.0, gamma=2.5, cutoff=0x18, remap=None, fps=60., framebuf=True):
        self.n = n
        if framebuf:
            self.buf = self.create_framebuffer()

        self.buf16 = self.create_framebuffer16()
        self.outbuf = uarray.array('H', (0 for _ in range(n*len(led_order) + 2)))

        mv = memoryview(self.outbuf)
        from struct import unpack
        mv[-2], mv[-1] = unpack('HH', b'\xff\xff\xff\xf0')

        self.framebuffer = mv[:-2]

        self.led_order = led_order
        self.gamma_map = uarray.array('H', (0 for _ in range(256)))
        self.remap = remap
        self.calc_gamma_map(gamma=gamma, cutoff=cutoff, brightness=brightness)
        #self.uart = machine.UART(1, baudrate=baudrate, rx=rx, tx=tx, txbuf=len(self.outbuf)*2)
        self.queue = _uartpixel.FrameQueue(uart=1, timer=3,
                                           baudrate=baudrate, rx=rx, tx=tx,
                                           fps=fps,
                                           framesize=len(self.outbuf)*2, framecount=12)
        self.reset()

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

    def reset(self):
        self.queue.push(b'\xff\xff\xff\xff\xf0') # send bad end of frame marker to make the stm32 ignore start-up gibberish

    def set_pulse_width(self, cycles):
        byte_width_in_cycles = int(cycles)*8
        lo, hi = byte_width_in_cycles&0xff, (byte_width_in_cycles>>8)&0xff
        self.reset()
        self.queue.push(bytes( (lo,hi,0xff,0xff,0xff,0xf3) ) )

    def write(self):
        self.writefrom(self.buf)

    def writefrom(self, buf):
        cball.framebuffer_8to16(self.buf16, buf)
        self.writefrom16(self.buf16)

    def writefromfloat(self, buf):
        cball.framebuffer_floatto16(self.buf16, buf)
        self.writefrom16(self.buf16)

    def writefrom16(self, buf16):
        cball.framebuffer_remap(self.framebuffer, buf16, self.remap, self.led_order)
        cball.framebuffer_gamma(self.framebuffer, self.gamma_map, self.cutoff)
        #self.uart.write(self.outbuf)
        self.queue.push(self.outbuf)

    def writeraw16(self, buf16):
        cball.framebuffer_remap(self.framebuffer, buf16, self.remap, self.led_order)
        self.queue.push(self.outbuf)
