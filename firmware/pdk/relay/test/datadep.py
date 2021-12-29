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

import sys
import pdk

USEC=1000
TRESET = 280*USEC
TPULSE = 1410
T0H = 375
T1H = 875
TSAMPLE = T1H-T0H
T0L = TPULSE-T0H
T1L = TPULSE-T1H

DATA = -1

def pulse(v):
    yield (1, T0H)
    yield (v, TSAMPLE)
    yield (0, T1L)

def wait(nsecs):
    yield (0, nsecs)

def wave_bits(n):
    for i in range(n):
        yield from pulse(DATA)

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
    yield from wait(TRESET)
    for i in range(3*8):
        yield from pulse(0)
    yield from msg_data(15, (1,1,1) )
    yield from wave_bits(60*8*3)
    yield from wait(10*TPULSE)

def sampler(wave_func, dt):
    try:
        t = 0

        v, ival = next(wave_func)
        assert(ival >= 0)
        t_in = ival

        while True:
            while t > t_in:
                v, ival = next(wave_func)
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
        if last != v and t != 0:
            yield (v, t)
            last = v
            t = 0
        t += dt
    yield (v, t)


def run(program, input_):

    output = []

    ctx_hi = pdk.new_ctx()
    ctx_lo = pdk.new_ctx()

    for i,x in enumerate(input_):
        t_hi = pdk.state_to_tuple(ctx_hi)
        t_lo = pdk.state_to_tuple(ctx_lo)
        a = int ( pdk.read_io(ctx_lo, 0x10) & 0x80 != 0 )
        b = int ( pdk.read_io(ctx_hi, 0x10) & 0x80 != 0 )
        if a == b:
            output.append(a)
        else:
            assert (a==0 and b==1)
            output.append(DATA)

        if x == DATA:
            pdk.set_pin(ctx_hi, 1<<3)
            pdk.set_pin(ctx_lo, 0<<3)
        else:
            pdk.set_pin(ctx_hi, x<<3)
            pdk.set_pin(ctx_lo, x<<3)

        pdk.step(program, ctx_hi)
        pdk.step(program, ctx_lo)

program = pdk.parse_program(open(sys.argv[1],'r').read())
run(program, sampler(waveform(), 119))
