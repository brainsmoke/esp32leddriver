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


with open(sys.argv[1]) as f:
    program = pdk.parse_program(f.read(), arch='pdk14')

verb = (sys.argv+[None])[2] == '-v'

ctx = pdk.new_ctx()

CHANNELS = [ 4, 3, 6 ]
UART_SIGNAL_BIT=0
HALF_0_BIT=1
HALF_1_BIT=2

def expected(t):
    ex = 0
    t_per = t&63
    n_per = t>>6
    channel = CHANNELS[n_per%3]
    pwm_len = 32-(n_per//3)%33
    if 0 <= t_per < 13:
        ex |= 1<<UART_SIGNAL_BIT

    if 14+7 <= t_per < 14+7+32:
        ex |= 1<<HALF_0_BIT
    else:
        ex |= 1<<HALF_1_BIT

    if 14+7+32-pwm_len <= t_per < 14+7+32:
        ex |= 1<<channel
    return ex

s=[]
lastleds=" "
while not pdk.read_io_raw(ctx, 0x10) & (1<<UART_SIGNAL_BIT):
    if verb:
        print(pdk.get_opcode(ctx, program))
    pdk.step(program, ctx)

t=0

MAX_T = 64*3*33

while True:
    pa = pdk.read_io_raw(ctx, 0x10)
    ex = expected(t)
    t_per = (t-13) % 64
    if verb:
      print(' '.join(' #'[int( pa&(1<<i) != 0)] for i in range (8)), "|",
            ' '.join(' #'[int( ex&(1<<i) != 0)] for i in range (8)), f"| {t_per:02d}", pdk.get_opcode(ctx, program))
    if ex != pa:
         print("Error")
         sys.exit(1)

    if t == MAX_T:
         print("Pass")
         sys.exit(0)
	
    pdk.step(program, ctx)
    t += 1
