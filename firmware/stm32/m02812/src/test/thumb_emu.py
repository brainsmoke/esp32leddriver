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
# Partial Cortex-M0 emulator
#
#
# LARGELY UNTESTED
#
#

import sys, subprocess

RAM_START=0x20000000
RAM_END=0x20001000

def from_le(arr):
    n=0
    for i,b in enumerate(arr):
        n+= b<<(8*i)
    return n

def to_le(n, size):
    arr = [0]*size
    for i in range(size):
        arr[i] = (n>>(i*8))&0xff
    return arr

def from_le_array(arr, elem_size):
    bytearr = [0]*(len(arr)*elem_size)
    for i,e in enumerate(arr):
        for j in range(elem_size):
            bytearr[i*elem_size+j] = (e>>(j*8))&0xff
    return bytearr

def to_le_array(bytearr, elem_size):
    arr = [0]*(len(bytearr)//elem_size)
    for i,e in enumerate(bytearr):
        j, shift = int(i//elem_size), (i%elem_size)*8
        arr[j] |= (e&0xff)<<shift
    return arr

#
# VM
#

def set_pc(ctx, pc):
    assert(not pc&1)
    ctx['r15'] = (pc+2)&0xffffffff

def get_pc(ctx):
    return ctx['r15']-4 #!!!

def stall(ctx, n_cycles):
    ctx['stall']+=n_cycles

def is_io(ctx, addr):
    return 0x40000000 <= addr <= 0x50000000

def read_mem(ctx, addr, size):
    mem = ctx['mem']
    return tuple( mem[addr+i] for i in range(size) )

def write_mem(ctx, addr, arr):
    if not (RAM_START <= addr <= addr+len(arr) <= RAM_END):
        raise AssertionError("write to read-only memory")
    mem = ctx['mem']
    for i,v in enumerate(arr):
        mem[addr+i] = v

def read_mem8(ctx, addr):
    if is_io(ctx, addr):
        return unsigned8(ctx['ioread'](ctx, addr, 8))
    else:
        return from_le(read_mem(ctx, addr, 1))

def read_mem16(ctx, addr):
    if addr&1 != 0:
        raise AssertionError("bad alignment")
    if is_io(ctx, addr):
        return unsigned16(ctx['ioread'](ctx, addr, 16))
    else:
        return from_le(read_mem(ctx, addr, 2))

def read_mem32(ctx, addr):
    if addr&3 != 0:
        raise AssertionError("bad alignment")
    if is_io(ctx, addr):
        return unsigned32(ctx['ioread'](ctx, addr, 32))
    else:
        return from_le(read_mem(ctx, addr, 4))

def write_mem8(ctx, addr, val):
    if is_io(ctx, addr):
        ctx['iowrite'](ctx, addr, unsigned8(val), 8)
    else:
        return write_mem(ctx, addr, to_le(val, 1))

def write_mem16(ctx, addr, val):
    if addr&1 != 0:
        raise AssertionError("bad alignment")

    if is_io(ctx, addr):
        ctx['iowrite'](ctx, addr, unsigned16(val), 16)
    else:
        return write_mem(ctx, addr, to_le(val, 2))

def write_mem32(ctx, addr, val):
    if addr&3 != 0:
        raise AssertionError("bad alignment")

    if is_io(ctx, addr):
        ctx['iowrite'](ctx, addr, val, 32)
    else:
        return write_mem(ctx, addr, to_le(val, 4))

def op_unimpl(ctx):
    raise AssertionError("unimplemented instruction")

def op_undef(ctx, **kwargs):
    print(hex(get_pc(ctx)))
    raise AssertionError("undefined instruction")

def op_shift(ctx, shiftop, dst, src, imm):
    ctx[dst], ctx['flags'] = shiftop(ctx[src], imm, ctx['flags'])

def op_arg3(ctx, alu, dst, src, src2):
    ctx[dst], ctx['flags'] = alu(ctx[src], ctx[src2], ctx['flags'])

def op_arg3i(ctx, alu, dst, src, imm):
    ctx[dst], ctx['flags'] = alu(ctx[src], imm, ctx['flags'])

def op_imm(ctx, alu, dst, imm):
    ctx[dst], ctx['flags'] = alu(ctx[dst], imm, ctx['flags'])

def op_alu(ctx, alu, dst, src):
    ctx[dst], ctx['flags'] = alu(ctx[dst], ctx[src], ctx['flags'])

def op_hi(ctx, alu, dst, src):
    ctx[dst], ctx['flags'] = alu(ctx[dst], ctx[src], ctx['flags'])

def op_ld_pcrel(ctx, dst, imm):
    ctx[dst] = read_mem32(ctx, (ctx['r15']&~3)+imm*4)
    stall(ctx, 1)

def op_ldst_regoff(ctx, dst, base, index, isload):
    if isload:
        ctx[dst] = read_mem32(ctx, ctx[base]+ctx[index])
    else:
        write_mem32(ctx, ctx[base]+ctx[index], ctx[dst])
    stall(ctx, 1)

def op_ldstb_regoff(ctx, dst, base, index, isload):
    if isload:
        ctx[dst] = read_mem8(ctx, ctx[base]+ctx[index])
    else:
        write_mem8(ctx, ctx[base]+ctx[index], ctx[dst])
    stall(ctx, 1)

def op_ldsth_regoff(ctx, dst, base, index, isload):
    if isload:
        ctx[dst] = read_mem16(ctx, ctx[base]+ctx[index])
    else:
        write_mem16(ctx, ctx[base]+ctx[index], ctx[dst])
    stall(ctx, 1)

def op_ldbsx_regoff(ctx, dst, base, index):
    ctx[dst] = signed8(read_mem8(ctx, ctx[base]+ctx[index]))&0xffffffff
    stall(ctx, 1)

def op_ldhsx_regoff(ctx, dst, base, index):
    ctx[dst] = signed16(read_mem16(ctx, ctx[base]+ctx[index]))&0xffffffff
    stall(ctx, 1)

def op_ldst_immoff(ctx, dst, base, imm, isload):
    if isload:
        ctx[dst] = read_mem32(ctx, ctx[base]+imm*4)
    else:
        write_mem32(ctx, ctx[base]+imm*4, ctx[dst])
    stall(ctx, 1)

def op_ldstb_immoff(ctx, dst, base, imm, isload):
    if isload:
        ctx[dst] = read_mem8(ctx, ctx[base]+imm)
    else:
        write_mem8(ctx, ctx[base]+imm, ctx[dst])
    stall(ctx, 1)

def op_ldsth_immoff(ctx, dst, base, imm, isload):
    if isload:
        ctx[dst] = read_mem16(ctx, ctx[base]+imm*2)
    else:
        write_mem16(ctx, ctx[base]+imm*2, ctx[dst])
    stall(ctx, 1)

def op_ldst_sprel(ctx, dst, imm, isload):
    if isload:
        ctx[dst] = read_mem32(ctx, ctx['r13']+imm*4)
    else:
        write_mem32(ctx, ctx['r13']+imm*4, ctx[dst])
    stall(ctx, 1)

def op_ldaddr(ctx, dst, src, imm):
    ctx[dst] = (ctx[src]+imm)&0xffffffff

def op_addsp(ctx, imm):
    ctx['r13'] = ctx['r13']+imm

def ldst_many(ctx, base_addr, reg_list, isload):
    for i,dst in enumerate(reg_list):
        if isload:
            ctx[dst] = read_mem32(ctx, (base_addr+4*i)&0xffffffff)
        else:
            write_mem32(ctx, (base_addr+4*i)&0xffffffff, ctx[dst])
    stall(ctx, len(reg_list))

def op_ldstm(ctx, base, reg_list, isload):
    ldst_many(ctx, ctx[base], reg_list, isload)
    if base not in reg_list:
        ctx[base] = (ctx[base]+4*len(reg_list))&0xffffffff

def op_pushpop(ctx, reg_list, isload):
    if not isload:
        ctx['r13'] = ( ctx['r13'] - 4*len(reg_list) )&0xffffffff
    ldst_many(ctx, ctx['r13'], reg_list, isload)
    if isload:
        ctx['r13'] = ( ctx['r13'] + 4*len(reg_list) )&0xffffffff

def op_bxx(ctx, cond, reladdr):
    if cond(ctx['flags']):
        op_b(ctx, reladdr)

def op_uxtb(ctx, dst, src):
    ctx[dst] = unsigned8(ctx[src])

def op_b(ctx, reladdr):
    set_pc(ctx, ctx['r15']+reladdr)
    stall(ctx, 2)
    if not -8 <= reladdr <= 0: # probably not precise, emperical
        stall(ctx, 3)

#

def unsigned32(a):
    return a&0xffffffff

def signed32(a):
    return (a&0x7fffffff)-(a&0x80000000)

def unsigned16(a):
    return a&0xffff

def signed16(a):
    return (a&0x7fff)-(a&0x8000)

def unsigned8(a):
    return a&0xff

def signed8(a):
    return (a&0x7f)-(a&0x80)

FLAGS_N = 1<<0
FLAGS_Z = 1<<1
FLAGS_C = 1<<2
FLAGS_V = 1<<3

def nzflags(v, flags):
    flags &=~ (FLAGS_N|FLAGS_Z)
    flags |=  (FLAGS_N*bool( v&0x80000000 )) | (FLAGS_Z*bool( v==0 ))
    return v&0xffffffff, flags

def alu_and(a, b, flags):
    return nzflags( a&b, flags )

def alu_eor(a, b, flags):
    return nzflags( a^b, flags )

def alu_lsl(a, b, flags):
    val = (a<<b)&0xffffffff
    c = bool( (a<<b)&0x100000000 )
    flags &=~ FLAGS_C
    flags |=  FLAGS_C*c
    return nzflags(val, flags)

def shift_right(a, b, flags):
    val = a>>b
    if b!=0:
         c = (a>>(b-1))&1
         flags &=~ FLAGS_C
         flags |=  FLAGS_C*c
    return nzflags(val, flags)

def alu_lsr(a, b, flags):
    return shift_right( unsigned32(a), b, flags )

def alu_asr(a, b, flags):
    return shift_right( signed32(a), b, flags )

def alu_adc(a, b, flags):
    c = bool(flags&FLAGS_C)
    val = a+b+c
    c = bool(val&0x100000000)
    flags &=~ FLAGS_C
    flags |=  FLAGS_C*c
    return nzflags(val, flags)

def alu_sbc(a, b, flags):
    c = bool(flags&FLAGS_C)
    val = a-b-(1-c)
    c = bool(val&0x100000000)
    v = val != signed32(val)
    flags &=~ (FLAGS_C|FLAGS_V)
    flags |=  (FLAGS_C*c) | (FLAGS_V*v)
    return nzflags(val, flags)

def alu_ror(a, b, flags):
    val = ( (a*0x100000001)>>(b&0x1f) ) & 0xffffffff
    c = bool(val&0x80000000)
    flags &=~ FLAGS_C
    flags |=  FLAGS_C*c
    return nzflags(val, flags)

def alu_tst(a, b, flags):
    _, flags = alu_and(a, b, flags)
    return a, flags

def alu_neg(a, b, flags):
    return alu_sub(0, b, flags)

def alu_cmp(a, b, flags):
    _, flags = alu_sub(a, b, flags)
    return a, flags

def alu_cmn(a, b, flags):
    _, flags = alu_add(a, b, flags)
    return a, flags

def alu_orr(a, b, flags):
    return nzflags( a|b, flags )

def alu_mul(a, b, flags):
    return nzflags( a*b, flags )

def alu_bic(a, b, flags):
    return nzflags( a & ~b, flags )

def alu_mvn(a, b, flags):
    return nzflags( ~b, flags )

def alu_mov(a, b, flags):
    return nzflags( b, flags )

def alu_add(a, b, flags):
    return alu_adc(a, b, flags & ~FLAGS_C)

def alu_sub(a, b, flags):
    return alu_sbc(a, b, flags | FLAGS_C)

def alu_add_noflags(a, b, flags):
    return (a+b)&0xffffffff, flags

def alu_mov_noflags(a, b, flags):
    return b&0xffffffff, flags

def cond_beq(flags):
    return bool(flags&FLAGS_Z)

def cond_bne(flags):
    return not cond_beq(flags)

def cond_bcs(flags):
    return bool(flags&FLAGS_C)

def cond_bcc(flags):
    return not cond_bcs(flags)

def cond_bmi(flags):
    return bool(flags&FLAGS_N)

def cond_bpl(flags):
    return not cond_bpl(flags)

def cond_bvs(flags):
    return bool(flags&FLAGS_V)

def cond_bvc(flags):
    return not cond_bvs(flags)

def cond_bhi(flags):
    return bool(flags&FLAGS_C) and not bool(flags&FLAGS_Z)

def cond_bls(flags):
    return not cond_bhi(flags)

def cond_bge(flags):
    return bool(flags&FLAGS_N) == bool(flags&FLAGS_V)

def cond_blt(flags):
    return not cond_bge(flags)

def cond_bgt(flags):
    return cond_bne(flags) and cond_bge(flags)

def cond_ble(flags):
    return not cond_bgt(flags)

# Optable parsing

alu_map = {
    '0000' : alu_and,
    '0001' : alu_eor,
    '0010' : alu_lsl,
    '0011' : alu_lsr,
    '0100' : alu_asr,
    '0101' : alu_adc,
    '0110' : alu_sbc,
    '0111' : alu_ror,
    '1000' : alu_tst,
    '1001' : alu_neg,
    '1010' : alu_cmp,
    '1011' : alu_cmn,
    '1100' : alu_orr,
    '1101' : alu_mul,
    '1110' : alu_bic,
    '1111' : alu_mvn,
}

alu_map_imm = {
    '00' : alu_mov,
    '01' : alu_cmp,
    '10' : alu_add,
    '11' : alu_sub,
}

alu_map_add_sub = {
    '0'  : alu_add,
    '1'  : alu_sub,
}

alu_map_hi = {
    '00' : alu_add_noflags,
    '01' : alu_cmp,
    '10' : alu_mov_noflags,
}

shift_op = {
    '00' : alu_lsl,
    '01' : alu_lsr,
    '10' : alu_asr,
}

cond_op = {
    '0000' : cond_beq,
    '0001' : cond_bne,
    '0010' : cond_bcs,
    '0011' : cond_bcc,
    '0100' : cond_bmi,
    '0101' : cond_bpl,
    '0110' : cond_bvs,
    '0111' : cond_bvc,
    '1000' : cond_bhi,
    '1001' : cond_bls,
    '1010' : cond_bge,
    '1011' : cond_blt,
    '1100' : cond_bgt,
    '1101' : cond_ble,
}

def identity(bitstring):
    return bitstring

def alu_parse(bitstring):
    return alu_map[bitstring]

def imm_alu_parse(bitstring):
    return alu_map_imm[bitstring]

def add_sub_parse(bitstring):
    return alu_map_add_sub[bitstring]

def hi_alu_parse(bitstring):
    return alu_map_hi[bitstring]

def shiftop_parse(bitstring):
    return shift_op[bitstring]

def cond_parse(bitstring):
    return cond_op[bitstring]

def number(bitstring):
    return int(bitstring,2)

def signed_number(bitstring):
    return int(bitstring[1:],2) - ( int(bitstring[0]=='1')<<(len(bitstring)-1) )

def jump_relative(bitstring):
    return signed_number(bitstring)<<1

def reg_parse(bitstring):
    if len(bitstring) == 3:
        bitstring = '0'+bitstring
    return 'r'+str(number(bitstring))

def sp_or_pc(bitstring):
    if bitstring == '0':
        return 'r15'
    elif bitstring == '1':
        return 'r13'

def reg_list(bitstring):
    return [ 'r'+str(i) for i in range(8) if (bitstring[7-i] == '1') ]

def boolean(bitstring):
    return bitstring == '1'

fields = {
    '?' : ('unknown',    identity      ),
    '.' : ('unknown',    identity      ),

    'U' : ('alu',        alu_parse     ),
    'a' : ('alu',        imm_alu_parse ),
    'A' : ('alu',        hi_alu_parse  ),
    'M' : ('alu',        add_sub_parse ),
    '<' : ('shiftop',    shiftop_parse ),

    'd' : ('dst',        reg_parse     ),
    's' : ('src',        reg_parse     ),
    'n' : ('src2',       reg_parse     ),
    'o' : ('index',      reg_parse     ),
    'b' : ('base',       reg_parse     ),
    'S' : ('src',        sp_or_pc      ),

    'R' : ('reg_list',   reg_list      ),

    'k' : ('imm',        number        ),
    'i' : ('imm',        signed_number ),
    'j' : ('reladdr',    jump_relative ),

    'L' : ('isload',     boolean       ),

    'c' : ('cond',       cond_parse    ),
}

opcodes = {
    '????????????????' :            op_unimpl,
    '000<<kkkkksssddd' : op_shift,
    '000110Mnnnsssddd' : op_arg3,
    '000111Mkkksssddd' : op_arg3i,
    '001aadddkkkkkkkk' : op_imm,
    '010000UUUUsssddd' : op_alu,
    '010001AAdssssddd' : op_hi,
    '010001110ssss...' :            op_unimpl, # op_bx,
    '010001111.......' :            op_undef,

    '010001AA1ssss111' :            op_unimpl, # PC
    '010001AAd1111ddd' :            op_unimpl, # PC

    '01001dddkkkkkkkk' : op_ld_pcrel,
    '0101L00ooobbbddd' : op_ldst_regoff,
    '0101L10ooobbbddd' : op_ldstb_regoff,
    '0101L01ooobbbddd' : op_ldsth_regoff,
    '0101011ooobbbddd' : op_ldbsx_regoff,
    '0101111ooobbbddd' : op_ldhsx_regoff,
    '0110Lkkkkkbbbddd' : op_ldst_immoff,
    '0111Lkkkkkbbbddd' : op_ldstb_immoff,
    '1000Lkkkkkbbbddd' : op_ldsth_immoff,
    '1001Ldddkkkkkkkk' : op_ldst_sprel,

    '1010Sdddkkkkkkkk' : op_ldaddr,
    '10110000iiiiiiii' : op_addsp,
    '1011L100RRRRRRRR' : op_pushpop,
#   '1011L101RRRRRRRR' : op_pushpop, # with PC,

    '1100LbbbRRRRRRRR' : op_ldstm,
    '1101ccccjjjjjjjj' : op_bxx,

    '1011001011sssddd' : op_uxtb,

#   '11011111kkkkkkkk' : op_swi,   # software interrupt
    '11100jjjjjjjjjjj' : op_b,
#   '1111HJJJJJJJJJJJ' : op_bl     # long branch
}

def parse_opcode(mem, addr, opcodes, fields):
    code = mem[addr] | mem[addr+1]<<8
    bitfield = '{:016b}'.format(code)
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
    return (op_func, 2, { fields[k][0]:fields[k][1](v) for k, v in field_patterns.items() })

def init_ctx(pc, code, mem, ioread=None, iowrite=None):
    return {
        'r0': None,
        'r1': None,
        'r2': None,
        'r3': None,
        'r4': None,
        'r5': None,
        'r6': None,
        'r7': None,
        'r8': None,
        'r9': None,
        'r10': None,
        'r11': None,
        'r12': None,
        'r13': None,
        'r14': None,
        'r15': pc+2,
        'flags': 0,
        'ioread' : ioread,
        'iowrite' : iowrite,
        'mem' : dict(mem),
        'code' : dict(code),
        'stall': 0,
        'ts': 0,
    }

def get_lines(argv):
    s = subprocess.run(argv, capture_output=True).stdout.decode('utf-8')
    return s.split('\n')[:-1]

def load_program(filename):

    mem = {}
    code = {}

    for line in get_lines( ['arm-none-eabi-objdump', '-s',
                            '--start-address='+str(0x8000000),
                            '--stop-address='+str(0x8800000), '--', filename] )[3:]:
        if line.startswith(' 8'):
            addr, data = int(line[0:8],16), bytes.fromhex(line[9:44])
            for i,v in enumerate(data):
                mem[addr+i]=v

    for i in mem:
        if not i&1 and i+1 in mem:            
            code[i] = parse_opcode(mem, i, opcodes, fields)

    symbols = { l.split(' ')[2]:int(l.split(' ')[0],16) for l in get_lines( ['nm', '--', filename] ) }

    return code, mem, symbols

print_fields = {
    'unknown':    str,
    'unknown':    str,
    
    'alu':       lambda x:x.__name__,
    'shiftop':   lambda x:x.__name__,
    
    'dst':       str,
    'src':       str,
    'src2':      str,
    'index':     str,
    'base':      str,
    'src':       str,
    
    'reg_list':  str,
    
    'imm':       str,
    'imm':       str,
    'reladdr':   str,
    
    'isload':    str,
    
    'cond':      str,
}


def print_regs(ctx):
    for i  in range(16):
        v = ctx['r'+str(i)]
        if v == None:
            v = '********'
        else:
            v = '{:8x}'.format(v)
        print(' {:4s}'.format('r'+str(i))+' '+v, end='')
        if i&7 == 7:
            print()
    print('flags:  {:8x}'.format(ctx['flags']))
        

def step(ctx):
    ctx['ts'] += 1
    if ctx['stall']:
        ctx['stall'] -= 1
    else:
        op_func, op_size, op_args = ctx['code'][ctx['r15']-2]
        #print_regs(ctx)
        #print (ctx['ts'], hex(ctx['r15']-2), op_func.__name__, ', '.join(k+': '+print_fields[k](v) for k,v in op_args.items()))
        #print (hex(ctx['r15']-2)[2:], op_func.__name__, ', '.join(k+': '+print_fields[k](v) for k,v in op_args.items()))
        ctx['r15'] += op_size
        op_func(ctx, **op_args)

def run(ctx, n_cycles=-1, end_pc=None):
    while (n_cycles==-1 or ctx['ts'] < n_cycles) and end_pc != ctx['r15']-2:
        step(ctx)

