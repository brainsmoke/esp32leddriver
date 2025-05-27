
TYPE_MAIN, TYPE_GLOBAL, TYPE_LOCAL = range(3)
TYPE_BITSHIFT = 2
def _btype(scope):
    assert scope in (TYPE_MAIN, TYPE_GLOBAL, TYPE_LOCAL)
    return scope<<TYPE_BITSHIFT

SIZE_0, SIZE_1, SIZE_2, SIZE_4 = range(4)
SIZE_BITSHIFT = 0
def _bsize(n):
    return { 0: SIZE_0, 1: SIZE_1, 2: SIZE_2, 4:SIZE_4 }[n]<<SIZE_BITSHIFT

SHORT_TAG_MAX = 15
TAG_MAX = 255
SHORT_TAG_LONGITEM = 15
SHORT_TAG_BITSHIFT = 4
def _btag(tag):
    assert 0 <= tag <= SHORT_TAG_MAX
    return tag<<SHORT_TAG_BITSHIFT

DATA_MAX = 256

def item(scope, tag, data):
    if data == None: # technically, a 0 could be encoded as size == 0 as well, don't assume support for this
        n_bytes = 0
    elif data < 0:
        n_bytes = (len(f"{-2*data:x}")+1)//2
    else:
        n_bytes = (len(f"{data:x}")+1)//2

    bytedata = list( ( data>>(i*8) ) & 0xff for i in range( n_bytes ) )

    if len(bytedata) == 3:
        bytedata.append( (data>>24) & 0xff )
    return item_bytedata( scope, tag, bytedata )

def item_bytedata(scope, tag, data):
    assert scope in (TYPE_MAIN, TYPE_GLOBAL, TYPE_LOCAL)
    assert tag <= TAG_MAX
    assert len(data) <= DATA_MAX

    prefix = bytearray(1)
    prefix[0] |= _btype(scope)
    if len(data) > 4 or tag >= SHORT_TAG_LONGITEM:
        prefix[0] |= _btag(SHORT_TAG_LONGITEM)
        prefix[0] |= _bsize(2)
        prefix.append(len(data))
        prefix.append(tag)
        assert "no long items are defined in the spec" == False
    else:
        prefix[0] |= _btag(tag)
        prefix[0] |= _bsize(len(data))

    return prefix + bytearray( data )


def strip_parens(s):
    assert s[0] == '(' and s[-1] == ')'
    return s[1:-1]

def lc(d):
    return { k.lower():v for k,v in d.items() }

input_bits = lc({
                # bit val
    "Data"     : ( 0, 0 ),
    "Const"    : ( 0, 1 ),
    "Constant" : ( 0, 1 ),

    "Arr"      : ( 1, 0 ),
    "Array"    : ( 1, 0 ),
    "Var"      : ( 1, 1 ),
    "Variable" : ( 1, 1 ),

    "Abs"      : ( 2, 0 ),
    "Absolute" : ( 2, 0 ),
    "Rel"      : ( 2, 1 ),
    "Relative" : ( 2, 1 ),

    "No Wrap" : ( 3, 0 ),
    "Wrap"    : ( 3, 1 ),

    "Linear"     : ( 4, 0 ),
    "Non Linear" : ( 4, 1 ),

    "Preferred State" : ( 5, 0 ),
    "No Preferred"    : ( 5, 1 ),
    "No Preferred State" : ( 5, 1 ),

    "No Null Position" : ( 6, 0 ),
    "Null State"       : ( 6, 1 ),

    "Bit Field"      : ( 8, 0 ),
    "Buffered Bytes" : ( 8, 1 ),
})

output_bits = input_bits.copy() | lc({

    "Non Volatile" : ( 7, 0 ),
    "Volatile" :     ( 7, 1 ),
})

feature_bits = output_bits.copy()

def parse_bitfield(s, field_def):
    bitnames = [ x.strip().lower() for x in strip_parens(s).split(',') ]
    ones = sum ( 1 << field_def[x][0] for x in bitnames if field_def[x][1] == 1 )
    zeroes = sum ( 1 << field_def[x][0] for x in bitnames if field_def[x][1] == 0 )
    if ones & zeroes != 0:
        raise ValueError(f"conflicting bitfield values: {s}")
    return ones

def parse_input_bits(s, ctx):
    return parse_bitfield(s, input_bits)

def parse_output_bits(s, ctx):
    return parse_bitfield(s, output_bits)

def parse_feature_bits(s, ctx):
    return parse_bitfield(s, feature_bits)

def parse_none(s, ctx):
    assert s == ''
    return None

collection = lc({
    "Physical"       : 0x00,
    "Application"    : 0x01,
    "Logical"        : 0x02,
    "Report"         : 0x03,
    "Named Array"    : 0x04,
    "Usage Switch"   : 0x05,
    "Usage Modifier" : 0x06,
})

def parse_collection(s, ctx):
    v = strip_parens(s).strip().lower()
    return collection[v]

def parse_unit(s, ctx):
    assert False

def parse_exponent(s, ctx):
    assert False

def parse_signed(s, ctx):
    s = strip_parens(s).lower()
    if s[-1] == 'h':
        return int(s[:-1], 16)
    elif s.startswith('0x'):
        return int(s[2:], 16)
    else:
        return int(s)

def parse_unsigned(s, ctx):
    n = parse_signed(s, ctx)
    assert n >= 0
    return n

def parse_usage_page(s, ctx):
    v = strip_parens(s).strip().lower()
    p = usage_page[v]
    ctx['page'] = p
    return p

def state_push(s, ctx):
    old_ctx = ctx.copy()
    ctx['old'] = old_ctx
    return parse_none(s, ctx)
    
def state_pop(s, ctx):
    old_ctx = ctx['old']
    for k in ctx:
        del ctx[k]
    for k in old_ctx:
        ctx[k] = old_ctx[k]
    return parse_none(s, ctx)


usage_page = {}
usages = {}

def new_usage_page(s, n, d):
    usage_page[s.lower()]=n
    usages[n] = lc(d)

new_usage_page("Generic Desktop", 0x01, {
    "Pointer"  : 0x01,
    "Mouse"    : 0x02,
    "Joystick" : 0x04,
    "Gamepad"  : 0x05,
    "Keyboard" : 0x06,
    "Keypad"   : 0x07,

    "X" : 0x30,
    "Y" : 0x31,
    "Z" : 0x32,

    "Wheel" : 0x38,

    "System Control"      : 0x80,
    "System Power Down"   : 0x81,
    "System Sleep"        : 0x82,
    "System Wake Up"      : 0x83,
    "System Cold Restart" : 0x8e,
    "System Warm Restart" : 0x8f,
})

new_usage_page("Button", 0x09, {

})


new_usage_page("LED", 0x08, {
	"Num Lock"    : 0x01,
	"Caps Lock"   : 0x02,
	"Scroll Lock" : 0x03,
	"Compose"     : 0x04,
	"Kana"        : 0x05,
})

new_usage_page("Consumer", 0x0c, {
    "Consumer Control" : 0x01,

    "Play"                : 0xb0,
    "Pause"               : 0xb1,
    "Record"              : 0xb2,
    "Fast Forward"        : 0xb3,
    "Rewind"              : 0xb4,
    "Scan Next Track"     : 0xb5,
    "Scan Previous Track" : 0xb6,
    "Stop"                : 0xb7,
    "Eject"               : 0xb8,

    "AL Calculator"   : 0x192,
    "AL Internet Browser"   : 0x196,
})

new_usage_page("Keyboard", 0x07, {

	"Volume Up"   : 0x80,
	"Volume Down" : 0x81,
})

def parse_usage(s, ctx):
    l = [ x.strip().lower() for x in strip_parens(s).split(',') ]
    assert 1 <= len(l) <= 2

    if len(l) == 1:
        hi = 0
        n = usages[ctx['page']].get(l[-1], None)
    else:
        hi = usage_page[l[0]]
        n = usages[hi].get(l[-1], None)

    if n == None:
        n = parse_signed('('+l[-1]+')', ctx)
    return n + hi*0x10000

items = lc({
    "Input"            : ( TYPE_MAIN,   0b1000, parse_input_bits ),
    "Output"           : ( TYPE_MAIN,   0b1001, parse_output_bits ),
    "Feature"          : ( TYPE_MAIN,   0b1011, parse_feature_bits ),
    "Collection"       : ( TYPE_MAIN,   0b1010, parse_collection ),
    "End Collection"   : ( TYPE_MAIN,   0b1100, parse_none ),
    "Usage Page"       : ( TYPE_GLOBAL, 0b0000, parse_usage_page ),
    "Logical Minimum"  : ( TYPE_GLOBAL, 0b0001, parse_signed ),
    "Logical Maximum"  : ( TYPE_GLOBAL, 0b0010, parse_signed ),
    "Physical Minimum" : ( TYPE_GLOBAL, 0b0011, parse_signed ),
    "Physical Maximum" : ( TYPE_GLOBAL, 0b0100, parse_signed ),
    "Unit Exponent"    : ( TYPE_GLOBAL, 0b0101, parse_exponent ),
    "Unit"             : ( TYPE_GLOBAL, 0b0110, parse_unit ),
    "Report Size"      : ( TYPE_GLOBAL, 0b0111, parse_unsigned ),
    "Report ID"        : ( TYPE_GLOBAL, 0b1000, parse_unsigned ),
    "Report Count"     : ( TYPE_GLOBAL, 0b1001, parse_unsigned ),
    "Push"             : ( TYPE_GLOBAL, 0b1010, state_push ),
    "Pop"              : ( TYPE_GLOBAL, 0b1011, state_pop ),
    "Usage"            : ( TYPE_LOCAL,  0b0000, parse_usage ),
    "Usage Minimum"    : ( TYPE_LOCAL,  0b0001, parse_usage ),
    "Usage Maximum"    : ( TYPE_LOCAL,  0b0010, parse_usage ),
#   ...

})


def parse_item(line, ctx):
    l = line.split('(', 1)
    name = l[0].strip().lower()
    if len(l) > 1:
        args = ('('+l[1]).strip()
    else:
        args = ''
    scope, tag, argfunc = items[name]
    return item(scope, tag, argfunc(args, ctx))
    

ctx = {}
import sys
for line in sys.stdin:
    if line.strip() == '':
        continue
    b = parse_item(line, ctx)
    l = ''.join(f'0x{x:02x},' for x in b )
    l += ' '*max(0, 16-len(l))
    print( '\t'+l+'/* '+line.strip()+' */')
    
