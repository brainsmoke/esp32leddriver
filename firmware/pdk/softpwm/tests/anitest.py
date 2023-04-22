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

import intelhex, pdk

BIT_UART = (1<<0)
CHANNELS = [ 4, 3, 6 ]

STOP_BITS=1
SAMPLES_PER_BYTE = (9+STOP_BITS)
RESET_BAUDS=100*3
program = []
ix = 0

offset = 3
uart_data = bytearray(bytes.fromhex(''.join(c for c in sys.argv[2] if c in '0123456789ABCDEFabcdef'))*9)

def uart_next(ctx):
    global ix

    byte = (ix // SAMPLES_PER_BYTE)
    if byte >= len(uart_data):
        val = BIT_UART
    else:
        bit = ix % SAMPLES_PER_BYTE
        val = None
        if bit == 0:
            val = 0
        elif bit > 8:
            val = BIT_UART
        else:
            val = BIT_UART*bool( uart_data[byte] & (1<<(bit-1)) )

    pdk.set_pin(ctx, val)

    ix += 1
    if ix >= len(uart_data)*SAMPLES_PER_BYTE+RESET_BAUDS:
        for i in range(4, len(uart_data), 6):
            uart_data[i] += 1
        print([int(c) for c in uart_data])
        ix = 0

with open(sys.argv[1]) as f:
    program = pdk.parse_program(f.read(), arch='pdk14')

ctx = pdk.new_ctx()

s=[]
t=0
lastleds=" "
while True:
    pa = pdk.read_io_raw(ctx, 0x10)
    pc = pdk.get_pc(ctx)

    leds = " 1234567"[sum( int(bool( pa & (1<<c)))<<i for i,c in enumerate(CHANNELS) )]
    if lastleds != leds:
#        print(pdk.prog_state(program, ctx))
        print ( f"{lastleds}:{t},")
        sys.stdout.flush()
        t=0
    lastleds = leds

    if pdk.get_opcode(ctx, program) in ('T0SN IO[0x010].0', 'T1SN IO[0x010].0'):
        uart_next(ctx)
    pdk.step(program, ctx)

    t += 1

