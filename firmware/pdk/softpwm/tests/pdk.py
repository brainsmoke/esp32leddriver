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
#

import sys

import intelhex

MEM_SIZE = 64

FLAGS         = 0x01
STACK_POINTER = 0x02

FLAG_Z, FLAG_C, FLAG_AC, FLAG_OV = ( 1<<x for x in range(4) )

MEMMAP       = { x:x for x in range(MEM_SIZE) }
PC_INDEX     = MEM_SIZE+0
A_INDEX      = MEM_SIZE+1
STALL_INDEX  = MEM_SIZE+2
IN_INDEX     = MEM_SIZE+7
IOREAD_INDEX = MEM_SIZE+8
FLAGS_INDEX  = MEM_SIZE+9
IOMAP        = { STACK_POINTER: MEM_SIZE+3, 0x10: MEM_SIZE+4, 0x11: MEM_SIZE+5, 0x12: MEM_SIZE+6, FLAGS: FLAGS_INDEX }
IOIGNORE     = (0x03, 0x0B, 0x3B)

def new_ctx():
    return [ 0 ] * 73 + ["BAD"]

def copy_ctx(ctx):
    return ctx[:]

def read_a(ctx):
    return ctx[A_INDEX]

def read_io(ctx, io):
    v = ctx[IOMAP[io]]
    if io == 0x10:
        iomask = ctx[IOMAP[0x11]]
        v &= iomask
        v |=~iomask & ctx[IN_INDEX]
        ctx[IOREAD_INDEX] = 1

    return v

def read_io_raw(ctx, io):
    return ctx[IOMAP[io]]

def xor_io(ctx, io, v):
    if io not in IOIGNORE:
        ctx[IOMAP[io]] ^= v

def get_pc(ctx):
    return ctx[PC_INDEX]

def read_mem(ctx, mem):
    return ctx[MEMMAP[mem]]

def read_mem16(ctx, mem):
    return ctx[MEMMAP[mem]]+256*ctx[MEMMAP[mem+1]]

def read_flags(ctx):
    return ctx[FLAGS_INDEX]

def write_a(ctx, a):
    ctx[A_INDEX] = a&0xff

def write_io(ctx, io, v):
    if io not in IOIGNORE:
        ctx[IOMAP[io]] = v

def write_mem(ctx, mem, v):
    ctx[MEMMAP[mem]] = v

def jump_to(ctx, addr):
    ctx[PC_INDEX] = addr
    ctx[STALL_INDEX] = 2

def stall(ctx):
    ctx[PC_INDEX] += 1
    ctx[STALL_INDEX] = 2

def skip_next(ctx):
    ctx[PC_INDEX] += 2
    ctx[STALL_INDEX] = 2

def stalled(ctx):
    return ctx[STALL_INDEX] != 0

def next_op(ctx):
    if ctx[STALL_INDEX]:
        ctx[STALL_INDEX] -= 1
    else:
        ctx[PC_INDEX] += 1

def write_flags(ctx, flags):
    ctx[FLAGS_INDEX] = flags

def set_pin(ctx, data):
    ctx[IN_INDEX] = data&0xff
    ctx[IOREAD_INDEX] = 0

def get_pout(ctx):
    return ctx[IOMAP[0x10]]&ctx[IOMAP[0x11]]

def ioread(ctx):
    return ctx[IOREAD_INDEX] != 0

def do_add(orig, d):
    flags = 0
    new = (orig+d)&0xff
    if new < orig:
        flags |= FLAG_C
    if new^0x80 < orig^0x80:
        flags |= FLAG_OV
    if new == 0:
        flags |= FLAG_Z
    if new&0xf < orig&0xf:
        flags |= FLAG_AC

    return (new, flags)

def do_sub(orig, d):
    flags = 0
    new = (orig-d)&0xff
    if new > orig:
        flags |= FLAG_C
    if new^0x80 > orig^0x80:
        flags |= FLAG_OV
    if new == 0:
        flags |= FLAG_Z
    if new&0xf > orig&0xf:
        flags |= FLAG_AC

    return (new, flags)

def un_addc(value, flags):
    if flags & FLAG_C:
        return ((value+1)&0xff, "BAD", 0)
    else:
        return (value, "BAD", 0)

def un_subc(value, flags):
    if flags & FLAG_C:
        return ((value-1)&0xff, "BAD", 0)
    else:
        return (value, "BAD", 0)

def un_izsn(value, flags):
    ret, flags = do_add(value, 1)
    return (ret, flags, ret == 0)

def un_dzsn(value, flags):
    ret, flags = do_sub(value, 1)
    return (ret, flags, ret == 0)

def un_inc(value, flags):
    ret, flags = do_add(value, 1)
    return (ret, flags, 0)

def un_dec(value, flags):
    ret, flags = do_sub(value, 1)
    return (ret, flags, 0)

def un_clear(value, flags):
    return (0, "BAD", 0)

def un_not(value, flags):
    return (value^0xff, "BAD", 0)

def un_neg(value, flags):
    return ( (-value)&0xff, "BAD", 0)

def un_sr(value, flags):
    return (value>>1, "BAD", 0)

def un_sl(value, flags):
    return ((value<<1)&0xff, "BAD", 0)

def un_src(value, flags):
    raise AssertionError("unimplemented")

def un_slc(value, flags):
    raise AssertionError("unimplemented")

def un_swap(value, flags):
    return ( (value>>4) | (value<<4) & 0xff, "BAD", 0)

def alu_add(dst, src, flags):
    ret, flags = do_add(dst, src)
    return (ret, flags)

def alu_sub(dst, src, flags):
    ret, flags = do_sub(dst, src)
    return (ret, flags)

def alu_addc(dst, src, flags):
    raise AssertionError("unimplemented")

def alu_subc(dst, src, flags):
    raise AssertionError("unimplemented")

def alu_and(dst, src, flags):
    return ( (dst&src), "BAD")

def alu_or(dst, src, flags):
    return ( (dst|src), "BAD")

def alu_xor(dst, src, flags):
    return ( (dst^src), "BAD")

def alu_mov(dst, src, flags):
    return (src, "BAD")

def op_unimpl(ctx):
    raise AssertionError("unimplemented")

def op_nop(ctx):
    return 0

def op_ldsptl(ctx, program):
    sp = read_io(ctx, STACK_POINTER)
    code_p = read_mem16(ctx, sp)
    opcode = program[code_p][2]
    tl = opcode & 0xff
    write_a(ctx, tl)
    stall(ctx)
    #print(hex(sp),hex(code_p),hex(tl))

def op_ldspth(ctx, program):
    sp = read_io(ctx, STACK_POINTER)
    code_p = read_mem16(ctx, sp)
    opcode = program[code_p][2]
    th = opcode >> 8
    write_a(ctx, th)
    stall(ctx)

def op_unary_a(ctx, un):
    a, flags, skip = un(read_a(ctx), read_flags(ctx))
    write_a(ctx, a)
    write_flags(ctx, flags)
    if skip:
        skip_next(ctx)

def op_pcadd_a(ctx):
    new_pc = get_pc(ctx)+read_a(ctx)
    if new_pc > 0x3ff:
        raise AssertionError("ub")
    jump_to(ctx, new_pc)

def op_wdreset(ctx):
    pass

def op_pushaf(ctx):
    raise AssertionError("unimplemented")

def op_popaf(ctx):
    raise AssertionError("unimplemented")

def op_reset(ctx):
    raise AssertionError("unimplemented")

def op_stopsys(ctx):
    raise AssertionError("unimplemented")

def op_stopexe(ctx):
    raise AssertionError("unimplemented")

def op_engint(ctx):
    raise AssertionError("unimplemented")

def op_disgint(ctx):
    raise AssertionError("unimplemented")

def op_ret(ctx):
    raise AssertionError("unimplemented")

def op_reti(ctx):
    raise AssertionError("unimplemented")

def op_xor_io_a(ctx, io):
    xor_io(ctx, io, read_a(ctx))

def op_mov_io_a(ctx, io):
    write_io(ctx, io, read_a(ctx))

def op_mov_a_io(ctx, io):
    write_a(ctx, read_io(ctx, io))
    write_flags(ctx, "BAD")

def op_stt16_m(ctx):
    raise AssertionError("unimplemented")

def op_ldt16_m(ctx):
    raise AssertionError("unimplemented")

def op_idxm_m_a(ctx, mem):
    addr = read_mem16(ctx, mem)
    a = read_a(ctx)
    write_mem(ctx, addr, a)
    stall(ctx)

def op_idxm_a_m(ctx, mem):
    addr = read_mem16(ctx, mem)
    a = read_mem(ctx, addr)
    write_a(ctx, a)
    stall(ctx)

def op_ret_k(ctx):
    raise AssertionError("unimplemented")

def op_test_m_bit_skip(ctx, mem, pos, boolean):
    if ( (read_mem(ctx, mem)>>pos)&1 ) == boolean:
        skip_next(ctx)

def op_set_m_bit(ctx, mem, pos, boolean):
    v = read_mem(ctx, mem)
    if boolean:
        v |= 1<<pos
    else:
        v &= ~(1<<pos)
    write_mem(ctx, mem, v)

def op_alu_m_a(ctx, alu, mem):
    m, flags = alu(read_mem(ctx, mem), read_a(ctx), read_flags(ctx))
    write_mem(ctx, mem, m)
    write_flags(ctx, flags)

def op_alu_a_m(ctx, alu, mem):
    a, flags = alu(read_a(ctx), read_mem(ctx, mem), read_flags(ctx))
    write_a(ctx, a)
    write_flags(ctx, flags)

def op_unary_m(ctx, un, mem):
    m, flags, skip = un(read_mem(ctx, mem), read_flags(ctx))
    write_mem(ctx, mem, m)
    write_flags(ctx, flags)
    if skip:
        skip_next(ctx)

def op_xch_m_a(ctx, mem):
    tmp = read_a(ctx)
    write_a(ctx, read_mem(ctx, mem))
    write_mem(ctx, mem, tmp)

def op_eq_m_skip(ctx, mem):
    if read_mem(ctx, mem) == read_a(ctx):
        skip_next(ctx)

def op_test_io_bit_skip(ctx, io, pos, boolean):
    if ( (read_io(ctx, io)>>pos)&1 ) == boolean:
        skip_next(ctx)

def op_set_io_bit(ctx, io, pos, boolean):
    v = read_io(ctx, io)
    if boolean:
        v |= 1<<pos
    else:
        v &= ~(1<<pos)
    write_io(ctx, io, v)

def op_alu_k(ctx, alu, imm):
    a, flags = alu(read_a(ctx), imm, read_flags(ctx))
    write_a(ctx, a)
    write_flags(ctx, flags)

def op_eq_k_skip(ctx, imm):
    if imm == read_a(ctx):
        skip_next(ctx)

def op_goto(ctx, addr):
    jump_to(ctx, addr)

def op_call(ctx, addr):
    raise AssertionError("unimplemented")

#
# Optable parsing
#

unary_op_map = {
    '0000' : un_addc,
    '0001' : un_subc,
    '0010' : un_izsn,
    '0011' : un_dzsn,
    '0100' : un_inc,
    '0101' : un_dec,
    '0110' : un_clear,
#   '0111' : ...
    '1000' : un_not,
    '1001' : un_neg,
    '1010' : un_sr,
    '1011' : un_sl,
    '1100' : un_src,
    '1101' : un_slc,
    '1110' : un_swap,
#   '0111' : ...
}

alu_map = {
    '000' : alu_add,
    '001' : alu_sub,
    '010' : alu_addc,
    '011' : alu_subc,
    '100' : alu_and,
    '101' : alu_or,
    '110' : alu_xor,
    '111' : alu_mov,
}

def number(bitstring):
    return int(bitstring,2)

def twice(bitstring):
    return 2*number(bitstring)

def unary_parse(bitstring):
    return unary_op_map[bitstring]

def alu_parse(bitstring):
    return alu_map[bitstring]

def identity(bitstring):
    return bitstring

fields = {
    '?' : ( 'unknown', identity    ),
    'u' : ( 'un',      unary_parse ),
    'a' : ( 'alu',     alu_parse   ),
    'i' : ( 'io',      number      ),
    'm' : ( 'mem',     number      ),
    'M' : ( 'mem',     twice       ),
    'B' : ( 'boolean', number      ),
    'k' : ( 'imm',     number      ),
    'j' : ( 'addr',    number      ),
    'b' : ( 'pos',     number      ),
}

opcodes_pdk13 = {

    '?????????????' : op_unimpl,
    '0000000000000' : op_nop,

    '0000000000110' : op_ldsptl,
    '0000000000111' : op_ldspth,

    '000000001uuuu' : op_unary_a,
    '00000000101??' : op_unimpl,
    '0000000010111' : op_pcadd_a,
    '0000000011111' : op_unimpl,

    '0000000110000' : op_wdreset,

    '0000000110010' : op_pushaf,
    '0000000110011' : op_popaf,

    '0000000110101' : op_reset,
    '0000000110110' : op_stopsys,
    '0000000110111' : op_stopexe,
    '0000000111000' : op_engint,
    '0000000111001' : op_disgint,
    '0000000111010' : op_ret,
    '0000000111011' : op_reti,
    '0000000111100' : op_unimpl,

    '00000011iiiii' : op_xor_io_a,
    '00000100iiiii' : op_mov_io_a,
    '00000101iiiii' : op_mov_a_io,

    '00000110MMMM0' : op_stt16_m,
    '00000110MMMM1' : op_ldt16_m,
    '00000111MMMM0' : op_idxm_m_a,
    '00000111MMMM1' : op_idxm_a_m,

    '00001kkkkkkkk' : op_ret_k,

    '00010bbbBmmmm' : op_test_m_bit_skip,
    '00011bbbBmmmm' : op_set_m_bit,

    '0010aaammmmmm' : op_alu_m_a,
    '0011aaammmmmm' : op_alu_a_m,
    '010uuuummmmmm' : op_unary_m,
    '0100111mmmmmm' : op_xch_m_a,
    '0101110mmmmmm' : op_eq_m_skip,
    '0101111??????' : op_unimpl,

    '0110Bbbbiiiii' : op_test_io_bit_skip,
    '0111Bbbbiiiii' : op_set_io_bit,

    '10aaakkkkkkkk' : op_alu_k,
    '10010kkkkkkkk' : op_eq_k_skip,
    '10011????????' : op_unimpl,

    '110jjjjjjjjjj' : op_goto,
    '111jjjjjjjjjj' : op_call,
}


opcodes_pdk14 = {

    '??????????????' : op_unimpl,
    '00000000000000' : op_nop,

    '00000000000110' : op_ldsptl,
    '00000000000111' : op_ldspth,

    '0000000110uuuu' : op_unary_a,
    '000000011001??' : op_unimpl,
    '00000001100111' : op_pcadd_a,
    '00000001101111' : op_unimpl,

    '00000001110000' : op_wdreset,

    '00000001110010' : op_pushaf,
    '00000001110011' : op_popaf,

    '00000001110101' : op_reset,
    '00000001110110' : op_stopsys,
    '00000001110111' : op_stopexe,
    '00000001111000' : op_engint,
    '00000001111001' : op_disgint,
    '00000001111010' : op_ret,
    '00000001111011' : op_reti,
    '00000001111100' : op_unimpl,

    '00000011iiiiii' : op_xor_io_a,
    '00000110iiiiii' : op_mov_io_a,
    '00000111iiiiii' : op_mov_a_io,

    '000010kkkkkkkk' : op_ret_k,
    '0000110MMMMMM0' : op_stt16_m,
    '0000110MMMMMM1' : op_ldt16_m,
    '0000111MMMMMM0' : op_idxm_m_a,
    '0000111MMMMMM1' : op_idxm_a_m,


    '0010aaammmmmmm' : op_alu_m_a,
    '0011aaammmmmmm' : op_alu_a_m,
    '010uuuummmmmmm' : op_unary_m,
    '0100111mmmmmmm' : op_xch_m_a,
    '0101110mmmmmmm' : op_eq_m_skip,
    '0101111???????' : op_unimpl,

    '0110Bbbbiiiiii' : op_test_io_bit_skip,
    '0111Bbbbiiiiii' : op_set_io_bit,

    '1000Bbbbmmmmmm' : op_test_m_bit_skip,
    '1001Bbbbmmmmmm' : op_set_m_bit,

    '101aaakkkkkkkk' : op_alu_k,
    '101010kkkkkkkk' : op_eq_k_skip,
    '101011????????' : op_unimpl,

    '110jjjjjjjjjjj' : op_goto,
    '111jjjjjjjjjjj' : op_call,

}

opcodes = {
    'pdk13' : opcodes_pdk13,
    'pdk14' : opcodes_pdk14,
}

opcode_len = { 'pdk13' : 13, 'pdk14' : 14 }

def parse_opcode(code, opcodes, opcode_len):
    bitfield = ('{:0'+str(opcode_len)+'b}').format(code)
    best = -1
    argbest = None
    for patt, op_func in opcodes.items():
        n_matches = 0
        field_patterns = {}
        for p,b in zip(patt,bitfield):
            if p == b:
                n_matches += 1
            elif p in '01' and b in '01':
                n_matches = -1
                break
            else:
                if p not in field_patterns:
                    field_patterns[p] = ''
                field_patterns[p] += b
        if n_matches > best:
            best = n_matches
            argbest = ( field_patterns, op_func )

    field_patterns, op_func = argbest
    return (op_func, { fields[k][0]:fields[k][1](v) for k, v in field_patterns.items() }, code)

def unary_str(t):
    return {
        un_addc:  "ADDC",
        un_subc:  "SUBC",
        un_izsn:  "IZSN",
        un_dzsn:  "DZSN",
        un_inc:   "INC",
        un_dec:   "DEC",
        un_clear: "CLEAR",
        un_not:   "NOT",
        un_neg:   "NEG",
        un_sr:    "SR",
        un_sl:    "SL",
        un_src:   "SRC",
        un_slc:   "SLC",
        un_swap:  "SWAP",
    }[t]

def alu_str(t):
    return {
        alu_add:  "ADD",
        alu_sub:  "SUB",
        alu_addc: "ADDC",
        alu_subc: "SUBC",
        alu_and:  "AND",
        alu_or:   "OR",
        alu_xor:  "XOR",
        alu_mov:  "MOV",
    }[t]

op_fmt = {
        op_unimpl:           "NOT IMPLEMENTED",
        op_nop:              "NOP",
        op_ldsptl:           "LDSPTL",
        op_ldspth:           "LDSPTH",
        op_unary_a:          "{un} A",
        op_pcadd_a:          "PCADD A",
        op_wdreset:          "WDRESET",
        op_pushaf:           "PUSH AF",
        op_popaf:            "POP AF",
        op_reset:            "RESET",
        op_stopsys:          "STOPSYS",
        op_stopexe:          "STOPEXE",
        op_engint:           "ENGINT",
        op_disgint:          "DISGINT",
        op_ret:              "RET",
        op_reti:             "RETI",
        op_xor_io_a:         "XOR {io}, A",
        op_mov_io_a:         "MOV {io}, A",
        op_mov_a_io:         "MOV A, {io}",
        op_stt16_m:          "STT16 {mem}",
        op_ldt16_m:          "LDT16_M {mem}",
        op_idxm_m_a:         "IDXM {mem}, A",
        op_idxm_a_m:         "IDXM A, {mem}",
        op_ret_k:            "RET {imm}",
        op_test_m_bit_skip:  "T{boolean}SN {mem}.{pos}",
        op_set_m_bit:        "SET{boolean} {mem}.{pos}",
        op_alu_m_a:          "{alu} {mem}, A",
        op_alu_a_m:          "{alu} A, {mem}",
        op_unary_m:          "{un} {mem}",
        op_xch_m_a:          "XCH {mem}, A",
        op_eq_m_skip:        "CEQSN A, {mem}",
        op_test_io_bit_skip: "T{boolean}SN {io}.{pos}",
        op_set_io_bit:       "SET{boolean} {io}.{pos}",
        op_alu_k:            "{alu} A, {imm}",
        op_eq_k_skip:        "CEQSN A, {imm}",
        op_goto:             "GOTO {addr}",
        op_call:             "CALL {addr}",
    }

def io_str(n):
    return "IO[0x{:03x}]".format(n)

def mem_str(n):
    return "MEM[0x{:03x}]".format(n)

def addr_str(n):
    return "0x{:03x}".format(n)

field_str = {
    'unknown': str,
    'un'     : unary_str,
    'alu'    : alu_str,
    'io'     : io_str,
    'mem'    : mem_str,
    'boolean': str,
    'imm'    : hex,
    'addr'   : addr_str,
    'pos'    : str,
}

def opcode_str(opcode):
    op, args, num = opcode
    return op_fmt[op].format( **{ k : field_str[k](v) for k,v in args.items() } )

def get_opcode(ctx, program):
    if stalled(ctx):
        return '[stall]'
    else:
        return opcode_str(program[get_pc(ctx)])

def get_in():
    return 0

def ctx_state(ctx, max_mem=MEM_SIZE):

    pa   = read_io_raw(ctx, 0x10)
    pac  = read_io_raw(ctx, 0x11)
    paph = read_io_raw(ctx, 0x12)

    def pin_chr(n):
        bit = 1<<n
        if not pac & bit:
            if paph & bit:
                return '^'
            else:
                return '?'
        elif pa & bit:
            return '#'
        else:
            return ' '

    s = '       '
    if stalled(ctx):
        s = '[stall]'

    #pc_a = 'PC: {:4x} A: {:02x}\n'.format(get_pc(ctx), read_a(ctx))
    #mem=''.join('{:04x}:   {:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x}   {:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x}\n'.format(x, *ctx[x:x+16]) for x in range(0, MEM_SIZE, 16) )
    #pins = '['+' '.join(pin_chr(x) for x in range(8) )+']\n\n'

    pc_a = 'PC: {:4x} A: {:02x} MEM: '.format(get_pc(ctx), read_a(ctx))
    mem=''.join('{:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x}  '.format(*ctx[x:x+8]) for x in range(0, max_mem, 8) )
    pins = '['+' '.join(pin_chr(x) for x in range(8) )+']'
    return pc_a+mem+pins

def prog_state(program, ctx, max_mem=MEM_SIZE):
    opstr = get_opcode( ctx, program )
    return opstr + ' '*(20-len(opstr)) + ctx_state(ctx, max_mem)

def state_to_tuple(ctx):
    return tuple(ctx[:-2])

def tuple_to_state(t):
    ctx = list(t)
    ctx.append(0)
    ctx.append(0)
    return ctx

def run(program, ctx, n_cycles=-1, callback=None):
    while n_cycles==-1 or ts < n_cycles:
        if callback and callback(program, ctx):
            break
        step(program, ctx)


def step(program, ctx):
    if not stalled(ctx):
        op_func, op_args, _ = program[get_pc(ctx)]
        if op_func in (op_ldsptl, op_ldspth):
            op_func(ctx, **op_args, program=program)
        else:
            op_func(ctx, **op_args)
    next_op(ctx)

def parse_program(s, arch='pdk13'):
    program = []
    mem = intelhex.parse(s)
    i = 0
    for i in range(0, max(mem.keys()), 2):
        if i in mem and i+1 in mem:
            opcode = mem[i] | mem[i+1]<<8
        else:
            opcode = 2**opcode_len[arch] - 1
        program.append( parse_opcode(opcode, opcodes[arch], opcode_len[arch]) )
        i+=2
    return program

