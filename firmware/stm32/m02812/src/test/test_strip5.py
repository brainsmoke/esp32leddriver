#!/usr/bin/env python3

import sys, random

import thumb_emu

PINMASK = 0x1f
T0L = 13
T1L = 33

def get_output_data(state, pulselength=60):
    data = []
    t0=last=None
    for t, val, pc in state:
        if t0 == None:
            if val == 0:
                continue
            assert val == PINMASK
            t0 = t
            last = t+T1L-pulselength

        ph = (t-t0)%pulselength
        ph_diff = t-last

        if (ph,ph_diff) not in ( (0,pulselength-T1L), (T0L,T0L), (T1L, T1L-T0L) ):
            print (t, val, hex(pc), ph, ph_diff)
            assert( False )

        if ph == 0:
            assert val == PINMASK
        if ph == T0L:
            assert val <= PINMASK
        if ph == T1L:
            assert val ==  0

        if ph == T0L:
            data += [val]

        last = t

    assert (state[-1][0]-t0)%pulselength == T1L

    return tuple(data)

def run_code(code, mem, symbols, remainders, buf, pulselength):
    GPIO = 0x48000014
    out = []
    def iowrite(ctx, addr, value, bits):
        assert addr == GPIO
        out.append( (ctx['ts'], value, thumb_emu.get_pc(ctx)) )

    ctx = thumb_emu.init_ctx(symbols['bitbang_start'], code, mem, iowrite=iowrite, ramsize=0x4000)
    ctx['r0'] = symbols['frame_a']
    ctx['r1'] = GPIO
    ctx['r2'] = pulselength*8
    ctx['r13'] = symbols['_estack']
    thumb_emu.write_mem(ctx, symbols['remainders'], remainders)
    thumb_emu.write_mem(ctx, symbols['frame_a'], thumb_emu.from_le_array(buf, 2))
    thumb_emu.run(ctx, end_pc = symbols['bitbang_end'])
    data = get_output_data(out, pulselength)
    remainders = thumb_emu.read_mem(ctx, symbols['remainders'], len(remainders))
    buf = thumb_emu.to_le_array(thumb_emu.read_mem(ctx, symbols['frame_a'], len(buf)*2), 2)

    assert ctx['r13'] == symbols['_estack']
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
    n_strips = 5
    remainders, buf = list(remainders), list(buf)
    values_per_strip = len(buf)//n_strips
    data = []
    for i in range(values_per_strip):
        m = [0]*8
        for j in range(n_strips):
            ix = i+j*values_per_strip
            v16 = buf[ix]+remainders[ix]
            remainders[ix] = v16&0xff
            m[j+3] = v16>>8

        data += scatter_gather(m)

    return tuple(data), tuple(remainders), tuple(buf)


def run_test(code, mem, symbols, remainders, buf, pulselength):
    d1, r1, b1 = run_algo(remainders, buf)
    d2, r2, b2 = run_code(code, mem, symbols, remainders, buf, pulselength)

    assert b1 == b2

    if d1 != d2:
        print(d1)
        print(d2)
        for i, e1, e2 in zip(range(len(d1)), d1, d2):
            print ("{} {} {}".format( i, e1, e2 ), end='')
            if e1 != e2:
                print (" ( {} ) <---".format(e1-e2), end='')
            print ()
        if len(d2) > len(d1):
            print ("excess data:")
            for i, e in enumerate(d2[len(d1):]):
                print("{} {}".format( i+len(d1), e ))
        if len(d2) < len(d1):
            print ("missing data:")
            for i, e in enumerate(d1[len(d2):]):
                print("{} {}".format( i+len(d2), e ))
        assert False

    assert r1 == r2

def run_tests(filename, n):

    code, mem, symbols = thumb_emu.load_program(filename)

    for n in range(n):
        remainders = [ random.randint(0, 0xff)   for x in range(symbols['VALUE_COUNT']) ]
        buf =        [ random.randint(0, 0xff00) for x in range(symbols['VALUE_COUNT']) ]

        run_test(code, mem, symbols, remainders, buf, 44+n%30)

filename = sys.argv[1]
run_tests(filename, 100)
