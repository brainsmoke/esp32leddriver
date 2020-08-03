
import machine, uarray

import cball

class UartPixel:

    def set_gamma(self, gamma, cutoff=0x18):
        max_val = 0xff00
        factor = max_val / (255.**gamma)
        self.gamma_map = bytearray(512)
        for i in range(256):
            v = int(factor * i**gamma)
            lo, hi = v&0xff, v>>8
            if lo <= cutoff//2:
                lo = 0
            elif lo < cutoff:
                lo = cutoff
            elif lo > 256-cutoff//2:
                lo, hi = 0, min(hi+1, max_val>>8)
            elif lo > 256-cutoff:
                lo = 256-cutoff
            self.gamma_map[i*2  ] = lo
            self.gamma_map[i*2+1] = hi

    def __init__(self, baudrate, rx, tx, n, gamma=2.5, cutoff=0x18, framebuf=True):
        self.n = n
        if framebuf:
            self.buf = bytearray(n*3)
        self.outbuf = bytearray(n*3*2 + 4)
        self.outbuf[-4:] = b'\xff\xff\xff\xf0'
        self.set_gamma(gamma, cutoff)
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
        cball.framebuf8to16(buf, self.outbuf, self.gamma_map)
        self.uart.write(self.outbuf)
