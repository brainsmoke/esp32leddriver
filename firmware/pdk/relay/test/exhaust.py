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

program = []

with open(sys.argv[1]) as f:
    mem = intelhex.parse(f.read())
    i = 0
    while i in mem and i+1 in mem:
        program.append(mem[i] | mem[i+1]<<8)
        i+=2

def cb(program, ctx):
    print (pdk.prog_state(program, ctx))

program = list(pdk.parse_opcode(x) for x in program)


t = pdk.state_to_tuple(pdk.new_ctx())
state_dict = { t:0 }
state_list = [t]
new_states = [t]
transitions = [ set() ]

while len(new_states) > 0:
    t0 = new_states.pop()
    i = state_dict[t0]
    for pin in range(256):
        ctx = pdk.tuple_to_state(t0)
        pdk.set_pin(ctx, pin)
        pdk.step(program, ctx)
        t = pdk.state_to_tuple(ctx)
        if t not in state_dict:
            transitions[i].add( len(state_list) )
            transitions.append( set() )
            state_dict[t] = len(state_list)
            state_list.append(t)
            new_states.append(t)
            if len(state_dict) % 1000 == 0:
                print (len(state_dict), file=sys.stderr)
        else:
            transitions[i].add( state_dict[t] )

        if not pdk.ioread(ctx):
            break

print (len(state_dict), file=sys.stderr)
states = state_list[:]
states.sort()
for t in states:
    ctx = pdk.tuple_to_state(t)
    print (pdk.prog_state(program, ctx))

