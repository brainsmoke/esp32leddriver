#!/usr/bin/env python3
#
# Copyright (c) 2020 Erik Bosman <erik@minemu.org>
#
# Permission  is  hereby  granted,  free  of  charge,  to  any  person
# obtaining  a copy  of  this  software  and  associated documentation
# files (the "Software"),  to deal in the Software without restriction,
# including  without  limitation  the  rights  to  use,  copy,  modify,
# merge, publish, distribute, sublicense, and/or sell copies of the
# Software,  and to permit persons to whom the Software is furnished to
# do so, subject to the following conditions:
#
# The  above  copyright  notice  and this  permission  notice  shall be
# included  in  all  copies  or  substantial portions  of the Software.
#
# THE SOFTWARE  IS  PROVIDED  "AS IS", WITHOUT WARRANTY  OF ANY KIND,
# EXPRESS OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR  PURPOSE  AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN
# CONNECTION  WITH THE SOFTWARE  OR THE USE  OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# (http://opensource.org/licenses/mit-license.html)
#

import sys,random
import pdk

USEC=1000000
TRESET = 280*USEC
TPULSE = 1500000
T0H = 375000
T1H = 875000
TSAMPLE = 550000
TDATA = T1H-T0H
T0L = TPULSE-T0H
T1L = TPULSE-T1H

T1L_MIN = 220000
T0H_MIN = 220000

DATA = -1

def pulse(v):
    yield (1, T0H)
    yield (v, TDATA)
    yield (0, T1L)

def wait(nsecs):
    yield (0, nsecs)

def wave_bits(n, d=DATA):
    for i in range(n):
        yield from pulse(d)

def skipped_pulse():
    yield from wait(TPULSE)

def static_bits( data ):
    for d in data:
        yield from pulse(d)

def static_msb4( n ):
    for d in ( 8, 4, 2, 1 ):
        yield from pulse(int( d&n != 0 ))

def msg_data( counter, pins ):
    yield from skipped_pulse()
    yield from static_msb4( counter )
    yield from static_bits( pins )

def waveform():
    yield from wait(TRESET*2)
    yield from wave_bits(1*8*3, 1)
    yield from wave_bits(3*8*3)
    for i in range(14):
        yield from msg_data(15-i, (1,1,1) )
        yield from wave_bits(1*8*3, i&1)
        yield from wave_bits(3*8*3)
    yield from wait(TRESET*2)

def coalesce(w):
    t=0
    cur = None
    for v, ival in w:
        if cur!=v:
            if t!=0:
                yield (cur, t)
            cur, t = v, 0
        t += ival
    else:
        yield (cur, t)

def decode(w):
    s=''
    w=coalesce(w)
    t_high = 0
    t_data = 0
    t_low = 0
    try:
        v, t_low = next(w)
        while True:
            assert v == 0
            assert t_low > T1L_MIN
            t_pulse = t_high+t_data+t_low
            if t_low > TRESET:
                s+='r'
            elif t_pulse > TPULSE * 3.5:
                s+='R'
            elif t_pulse > TPULSE * 1.5:
                s+='_'
            elif t_pulse < TPULSE-1e12/8e6:
                s+='<'
            v, t_high = next(w)
            assert v==1
            assert t_high > T0H_MIN
            assert t_high < TPULSE
            v, t = next(w)
            t_data = 0
            if v == DATA:
                t_data = t
                v, t_low = next(w)
                if t_high > TSAMPLE:
                    s+='E'
                elif t_high+t_data > TSAMPLE:
                    s+='d'
                else:
                    s+='X'
            else:
                assert v == 0
                if t_high > TSAMPLE:
                    s+='1'
                else:
                    s+='0'
                t_low = t
    except StopIteration:
        pass
    return s

def sampler(wave_func, dt):
    w = iter(wave_func)
    try:
        t = 0

        v, ival = next(w)
        assert(ival >= 0)
        t_in = ival

        while True:
            while t > t_in:
                v, ival = next(w)
                assert(ival >= 0)
                t_in += ival
            yield v
            t += dt

    except StopIteration:
        pass

def wave(samples, dt):
    t = 0
    last = None
    for v in samples:
        if last != v:
            if t != 0:
                yield (last, t)
            last = v
            t = 0
        t += dt
    yield (v, t)


def run(program, input_,d=False):

    output = []

    ctx_hi = pdk.new_ctx()
    ctx_lo = pdk.new_ctx()
    q=0
    input_equal = True
    reset=0

    for i,x in enumerate(input_):
        t_hi = pdk.state_to_tuple(ctx_hi)
        t_lo = pdk.state_to_tuple(ctx_lo)
        if t_hi == t_lo:
            q=0
        else:
            q+=1
        a = int ( pdk.read_io(ctx_lo, 0x10) & 0x80 != 0 )
        b = int ( pdk.read_io(ctx_hi, 0x10) & 0x80 != 0 )
        if a == b:
            c=a
        else:
            c=DATA

        output.append(c)

        if d:
            if t_hi == t_lo:
                 print ( ' #?'[x], ' #?'[c], pdk.prog_state(program, t_lo) )
            else:
                 print ( ' #?'[x], ' #?'[c], pdk.prog_state(program, t_lo), pdk.prog_state(program, t_hi) )

        if (x==DATA and input_equal and t_hi != t_lo) or (reset < 1 and c!=0):
            if not d:
                assert(False)
            else:
                return

        input_equal = x != DATA
        reset += int(pdk.get_pc(t_lo) == 0xc)

        if x == DATA:
            pdk.set_pin(ctx_hi, 1<<3)
            pdk.set_pin(ctx_lo, 0<<3)
        else:
            pdk.set_pin(ctx_hi, x<<3)
            pdk.set_pin(ctx_lo, x<<3)

        pdk.step(program, ctx_hi)
        pdk.step(program, ctx_lo)
    return output

program = pdk.parse_program(open(sys.argv[1],'r').read())

def fmt_samples(samples):
    s=''.join('_#?'[s] for s in samples)
    s=s.replace("_#","_|#").replace("_?","_|?")
    l=s.split('|')
    for i in range(0,len(l),12):
        print(''.join("{:15s}".format(x) for x in l[i:i+16]))

#for q in range(1424050,1414400,-100):
#for q in range(1457050,1414400,-100):
#for q in range(1414400,1499700, 100):
#for q in range(1457050,1499700, 100):
q=1457050
while True:
    print (q)
    TPULSE=q
    T0L = TPULSE-T0H
    T1L = TPULSE-T1H
    w = list(waveform())
    #print(decode(w))
    for n,i in enumerate(random.randint(125000-125*5, 125000+125*5) for _ in range(15)):
        samples = list(sampler(w, i))
        try:
            output = run(program, samples)
        except AssertionError:
            print (q,n)
            #print("PRE: ",fmt_samples(samples))
            #print("POST: ",fmt_samples(output))
            #run(program, samples, True)
            break
        w = list(wave(output, i))
        if not decode(w).startswith('r'+str(n&1)*24+'d'*(24*3)):
            #run(program, samples, True)
            #print(n, decode(w))
            print (q,n)
            break
