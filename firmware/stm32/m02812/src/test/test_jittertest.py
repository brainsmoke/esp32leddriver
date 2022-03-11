#!/usr/bin/env python3

import sys, random

import thumb_emu

def get_output_data(state, init=0):
    v = init
    last=None
    out = []
    for t, val, pc in state:
        if val != v:
            if last != None:
                yield (t-last, v)

            v = val
            last = t
    yield (-1, v)

def pulses(it, tup=(18,24,18)):
    d = { 'LLL': 'X', 'HLL': '0', 'HHL': '1' }
    ix = 0
    out = ''
    for t, v in it:
        if t == -1:
            while len(out) < len(tup):
                out += "LH"[int(v!=0)]
            yield d[out]
            break
        else:
            while t > 0:
                assert t >= tup[ix]
                out += "LH"[int(v!=0)]
                t -= tup[ix]
                ix += 1
                if ix == len(tup):
                    ix = 0
                    yield d[out]
                    out = ''

def decode(it):
    d = ''
    for x in it:
        d += x
        if len(d) == 8:
            if d[0] == 'X':
                yield 'route: ' + ' A'[d[7]=='1'] + ' B'[d[6]=='1'] +' C'[d[5]=='1'] + hex(int(d[1:5],2))
            else:
                yield hex(int(d, 2))
            d=''

def run_code(code, mem, symbols, frame, histogram):
    GPIOA     = 0x48000000
    GPIOA_ODR = GPIOA|0x14
    GPIOA_BRR = GPIOA|0x28
    TIM3      = 0x40000400
    TIM3_SR   = TIM3|0x10
    TIM3_CCR1 = TIM3|0x34
    out = []
    def iowrite(ctx, addr, value, bits):
        if addr == GPIOA_ODR:
            out.append( (ctx['ts'], value, thumb_emu.get_pc(ctx)) )
        else:
            assert False

    i = 0
    ccr1 = 0
    def ioread(ctx, addr, bits):
        nonlocal i, ccr1
        if addr == TIM3_SR:
            return 0x1d
        elif addr == TIM3_CCR1:
            i += 1
            if int(i % 4 == 0):
                ccr1=ctx['ts']
            thumb_emu.stall(ctx, 2)
            return ccr1
        elif addr == GPIOA_BRR:
            if bits <= 16:
                return 0
        print (hex(ctx['r15']), hex (addr), bits)
        assert False

    ctx = thumb_emu.init_ctx(symbols['bitbang_start'], code, mem, ioread=ioread, iowrite=iowrite)
    ctx['r0'] = symbols['frame']
    ctx['r1'] = 4
    ctx['r2'] = GPIOA
    ctx['r3'] = TIM3
    ctx['r13'] = symbols['_estack']
    thumb_emu.write_mem(ctx, symbols['frame'], thumb_emu.from_le_array(frame, 2))
    thumb_emu.write_mem(ctx, symbols['histogram'], thumb_emu.from_le_array(histogram, 4))
    thumb_emu.run(ctx, end_pc = symbols['bitbang_end'])
    #for tup in decode(pulses(get_output_data(out), (18,24,28) )):
    for tup in get_output_data(out):
        print(tup)
    get_output_data(out)
    histogram = thumb_emu.to_le_array(thumb_emu.read_mem(ctx, symbols['histogram'], len(histogram)*4), 4)
    for i,v in enumerate(histogram):
        if i != 0 and v != 0:
            print(i,v)
    return histogram


def run_tests(filename):

    code, mem, symbols = thumb_emu.load_program(filename)

    histogram = [ 0 ] * 256

    ROUTING = 0x000
    VALUE = 0x100
    def frame_value(v):
        return (v&0xff) | VALUE

    frame = [ frame_value(0xaa) ] * symbols['FRAME_LENGTH']

    def frame_route(mask, timeout):
        return mask|(timeout<<3) | ROUTING;

    def frame_set_rgb(index, r, g, b):
        frame[index+0] = frame_value(r)
        frame[index+1] = frame_value(g)
        frame[index+2] = frame_value(b)

    frame_set_rgb(0, 0x0f, 0x07, 0x00);
    frame_set_rgb(4, 0x0f, 0x00, 0x07);
    frame_set_rgb(8, 0x07, 0x0f, 0x00);
    frame_set_rgb(12, 0x07, 0x00, 0x0f);

    frame[3]= frame_route(1, 0);
    frame[7]= frame_route(3, 0);
    frame[11]= frame_route(7, 15);

    data = run_code(code, mem, symbols, frame, histogram)

filename = sys.argv[1]
run_tests(filename)
