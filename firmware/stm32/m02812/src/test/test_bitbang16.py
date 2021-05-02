#!/usr/bin/env python3

import sys, random

import thumb_emu

def get_output_data(state):
    data = []
    t0=last=None
    for t, val, pc in state:
        if t0 == None:
            if val == 0:
                continue
            assert val == 255
            t0 = t
            last = t-18

        ph = (t-t0)%60
        ph_diff = t-last

        #print (t, val, hex(pc), ph, ph_diff)
        if (ph,ph_diff) not in ( (0,18), (18,18), (42, 24) ):
            print (t, val, hex(pc), ph, ph_diff)
            assert( False )

        assert ph !=  0 or val == 255
        assert ph != 18 or val <= 255
        assert ph != 42 or val ==   0

        if ph == 18:
            data += [val]

        last = t

    assert (state[-1][0]-t0)%60 == 42

    return tuple(data)

def run_code(code, mem, symbols, remainders, buf):
    GPIO = 0x48000014
    out = []
    def iowrite(ctx, addr, value, bits):
        assert addr == GPIO
        out.append( (ctx['ts'], value, thumb_emu.get_pc(ctx)) )

    ctx = thumb_emu.init_ctx(symbols['bitbang_start'], code, mem, iowrite=iowrite)
    ctx['r0'] = symbols['frame_a']
    ctx['r1'] = GPIO
    thumb_emu.write_mem(ctx, symbols['remainders'], remainders)
    thumb_emu.write_mem(ctx, symbols['frame_a'], thumb_emu.from_le_array(buf, 2))
    thumb_emu.run(ctx, end_pc = symbols['bitbang_end'])
    data = get_output_data(out)
    remainders = thumb_emu.read_mem(ctx, symbols['remainders'], len(remainders))
    buf = thumb_emu.to_le_array(thumb_emu.read_mem(ctx, symbols['frame_a'], len(buf)*2), 2)
    return tuple(data), tuple(remainders), tuple(buf)

def scatter_gather(m):
    n = [0]*8
    for i,val in enumerate(m):
        assert 0 <= val <= 255
        for j in range(8):
            if val & (1<<(7-j)):
                n[j] |= (1<<(7-i))
    return tuple(n)

def run_algo(remainders, buf):
    remainders, buf = list(remainders), list(buf)
    leds_per_strip = len(buf)//8
    data = []
    for i in range(leds_per_strip):
        m = [0]*8

        for j in range(8):
            ix = i+j*leds_per_strip
            v16 = buf[ix]+remainders[ix]
            remainders[ix] = v16&0xff
            m[j] = v16>>8

        data += scatter_gather(m)

    return tuple(data), tuple(remainders), tuple(buf)


def run_test(code, mem, symbols, remainders, buf):
    d1, r1, b1 = run_algo(remainders, buf)
    d2, r2, b2 = run_code(code, mem, symbols, remainders, buf)

    assert b1 == b2

    if d1 != d2:
        for i, e1, e2 in zip(range(len(d1)), d1, d2):
            print (i, bin(e1), bin(e2), end='')
            if e1 != e2:
                print (" <---", end='')
            print ()
        assert False

    assert r1 == r2

def run_tests(filename, n):

    code, mem, symbols = thumb_emu.load_program(filename)

    for _ in range(n):
        remainders = [ random.randint(0, 0xff)   for x in range(symbols['VALUE_COUNT']) ]
        buf =        [ random.randint(0, 0xff00) for x in range(symbols['VALUE_COUNT']) ]

        run_test(code, mem, symbols, remainders, buf)

filename = sys.argv[1]
run_tests(filename, 100)
