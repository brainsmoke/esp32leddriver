
#
# file format:
#
# header:
# b'\1RANDOMWALK' + uint8(n_choices_per_step) + uint16(n_bias_rows) + uint16(n_positions)
#
# bias rows:
# ( [ uint8(weighing_choice_x) * n_choices_per_step ] * n_bias_rows
#
# position transitions:
# ( [ uint16(pos) * n_choices_per_step ] * n_position_rows
#
# bias transitions:
# ( [ uint16(bias_row_index) * n_choices_per_step ] * n_position_rows
#
# end
#

import sys, array, os, json

leds = [x['position'] for x in json.load(open(os.path.dirname(sys.argv[0])+'./../../firmware/esp32/models/greatcircles/leds.json', "r"))['leds']]

def v_add(a,b):
    return (a[0]+b[0], a[1]+b[1], a[2]+b[2])

leds = [ v_add(leds[i], leds[i+1]) for i in range(0, len(leds), 2) ] # treat opposite leds as mirrors

N_CHOICES_PER_STEP = 4
N_BIAS_ROWS = 2
N_ROWS = 120

MOVE_FW, MOVE_BW, MOVE_TURN_SHARP, MOVE_TURN_WIDE = 0, 1, 2, 3

def d(a, b):
    ax,ay,az = a
    bx,by,bz = b
    return ( (ax-bx)**2 + (ay-by)**2 + (az-bz)**2 ) **.5

d_list = [ d(leds[0], l) for l in leds ]
d_list.sort()
d_sharp, d_wide, d_straight = d_list[1:4]

neighbours = [ [None]*4 for i in range(len(leds)) ]

for a,pos_a in enumerate(leds):
    neighbours[a][MOVE_FW] == a^1
    for b,pos_b in enumerate(leds):
        dist = d(pos_a, pos_b)
        if d_sharp*.99 < dist < d_sharp*1.01:
            neighbours[a][MOVE_TURN_SHARP] = b
        elif d_wide*.99 < dist < d_wide*1.01:
            neighbours[a][MOVE_TURN_WIDE] = b
        elif d_straight*.99 < dist < d_straight*1.01:
            if a == b^1:
                neighbours[a][MOVE_BW] = b
            else:
                neighbours[a][MOVE_FW] = b

out = open(sys.argv[1], "wb")

header = b'\1RANDOMWALK' + bytes( (N_CHOICES_PER_STEP, N_BIAS_ROWS&0xff, N_BIAS_ROWS>>8, N_ROWS&0xff, N_ROWS>>8) )

out.write(header)

WEIGHT_FORWARD = 13
WEIGHT_TURN_SHARP = 1
WEIGHT_TURN_WIDE = 2
WEIGHT_BACK = 0


bias_rows = bytes( [

WEIGHT_FORWARD, WEIGHT_BACK, WEIGHT_TURN_SHARP, WEIGHT_TURN_WIDE,       # intermediate nodes
WEIGHT_BACK, WEIGHT_FORWARD, WEIGHT_BACK, WEIGHT_BACK,

] )

BIAS_FW, BIAS_BW = range(2)

turn_table = [[BIAS_BW, BIAS_FW, BIAS_BW, BIAS_BW]] * 120

assert len(neighbours) == N_ROWS
for n in neighbours:
    assert len(n) == N_CHOICES_PER_STEP

assert len(turn_table) == N_ROWS
for t in turn_table:
    assert len(t) == N_CHOICES_PER_STEP

out.write( bias_rows )
out.write( array.array("H", (e for row in neighbours for e in row ) ) )
out.write( array.array("H", (e for row in turn_table for e in row ) ) )


