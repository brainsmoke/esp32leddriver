#!/usr/bin/env python3
#
# Copyright (c) 2023 Erik Bosman <erik@minemu.org>
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

TRY_CYCLES_MAX = 10000

def get_settings_addr(filedata):
    ctx = pdk.new_ctx()
    program = pdk.parse_program(filedata, arch=arch)

    for i in range(TRY_CYCLES_MAX):
        op = pdk.get_opcode(ctx, program)
        pdk.step(program, ctx)
        if op == 'LDSPTL':
            return (pdk.read_stack_top_word(ctx), pdk.read_a(ctx))
    else:
        raise Exception("no LDSPTL near the start of execution")

def read_word(m, byteaddr, default=0xffff):
    if byteaddr & 1:
        raise ValueError("alignment error")

    if byteaddr in m:
        lo = m[byteaddr]
    else:
        lo = default & 0xff

    if byteaddr+1 in m:
        hi = m[byteaddr+1]<<8
    else:
        hi = default & 0xff00

    return hi|lo

def write_word(m, byteaddr, data):
    m[byteaddr+0] = data&0xff
    m[byteaddr+1] = (data>>8)&0xff


if __name__ == '__main__':

    arch = sys.argv[1]
    filename = sys.argv[2]
    command = sys.argv[3]

    if command not in ('get_address', 'set_address', 'change_address'):
        raise ValueError("command not in ('get_address', 'set_address', 'change_address')")

    if command in ('set_address', 'change_address'):
        index = int(sys.argv[4])
        if not 0 <= index <= 85:
             raise ValueError("not 0 <= index <= 85")

    if arch not in ('pdk13', 'pdk14'):
        raise ValueError("arch not in ('pdk13', 'pdk14')")

    unset = { 'pdk13':0x1fff, 'pdk14':0x3fff }[arch]
    set_hi = { 'pdk13':0x1700, 'pdk14':0x2f00 }[arch]

    with open(filename) as f:
        filedata = f.read()

    rom = intelhex.parse(filedata)

    settings_codeptr, led_addr = get_settings_addr(filedata)
    settings_codeptr *= 2 # hexfile uses byte addressing

    assert read_word(rom, settings_codeptr, default=unset) == ( set_hi | led_addr )

    if command == 'get_address':
        if led_addr % 3 != 0:
            raise Exception("address not a multiple of 3")
        print(led_addr//3)
        sys.exit()

    if command == 'change_address':
        m = {}
    else:
        rom[settings_codeptr] = 0xff
        m = rom

    cur_opcode = read_word(rom, settings_codeptr, default=unset)
    next_opcode = read_word(rom, settings_codeptr+2, default=unset)

    replacement_opcode = ( set_hi | (index * 3) )

    if cur_opcode == replacement_opcode:
        pass
    elif cur_opcode & replacement_opcode == replacement_opcode:
        write_word(m, settings_codeptr, replacement_opcode)
    else:
        if next_opcode != unset:
            raise ValueError("next opcode not unset {:04x}".format(next_opcode))
        write_word(m, settings_codeptr, cur_opcode&0xff)
        write_word(m, settings_codeptr+2, replacement_opcode)

    print(intelhex.generate(m), end='')

    sys.exit(0)

