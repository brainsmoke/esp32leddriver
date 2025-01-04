
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

conf = {

    'normal' : {
        'filename': './../../firmware/esp32/models/chiral_tiling/leds.json',
        'segment_count': 30,
    },
    'big' : {
        'filename': './../../firmware/esp32/models/chiral_big/leds.json',
        'segment_count': 30,
    },

}[sys.argv[1]]


leds = [x['position'] for x in json.load(open(os.path.dirname(sys.argv[0])+conf['filename'], "r"))['leds']]

N_CHOICES_PER_STEP = 3
N_BIAS_ROWS = 8 # 2 for leds with 2 neighbours, 3 for each intersection
N_ROWS = len(leds)
N_SEGMENTS = int(conf['segment_count'])

N_LEDS_PER_STRIP=N_ROWS//N_SEGMENTS
N_LEDS_PER_ARC=N_LEDS_PER_STRIP+2

assert N_LEDS_PER_STRIP*N_SEGMENTS == N_ROWS
assert (N_LEDS_PER_ARC-4)%3 == 0

SPLIT_A = (N_LEDS_PER_ARC-4)//3
SPLIT_B = SPLIT_A + 1 + SPLIT_A
START = 0
END = N_LEDS_PER_STRIP-1

MOVE_FW, MOVE_BW, MOVE_TURN = 0, 1, 2

def d(a, b):
    ax,ay,az = a
    bx,by,bz = b
    return ( (ax-bx)**2 + (ay-by)**2 + (az-bz)**2 ) **.5


d_expected = d(leds[0], leds[1])
 
NO_OPTION = 0xffff
neighbours = [ [NO_OPTION]*3 for _ in range(len(leds)) ]

def strip_index(x):
    return x % N_LEDS_PER_STRIP

def is_start_of_strip(x):
    return strip_index(x) == START

def is_neighbour(a, b):
    return d_expected*.99 < d(leds[a], leds[b]) < d_expected*1.01

for i in range(N_ROWS):
    if not is_start_of_strip(i):
        neighbours[i-1][MOVE_FW] = i
        neighbours[i]  [MOVE_BW]   = i-1

for strip_a in range(0, N_ROWS, N_LEDS_PER_STRIP):
    for strip_b in range(0, N_ROWS, N_LEDS_PER_STRIP):
        for a in (strip_a+START, strip_a+END):
            for b in (strip_b+SPLIT_A, strip_b+SPLIT_B):
                if is_neighbour(a, b):
                    if is_start_of_strip(a):
                        neighbours[a][MOVE_BW] = b
                    else:
                        neighbours[a][MOVE_FW] = b

                    neighbours[b][MOVE_TURN] = a

out = open(sys.argv[2], "wb")

header = b'\1RANDOMWALK' + bytes( (N_CHOICES_PER_STEP, N_BIAS_ROWS&0xff, N_BIAS_ROWS>>8, N_ROWS&0xff, N_ROWS>>8) )

out.write(header)

WEIGHT_FORWARD = 5
WEIGHT_TURN_SMALL = 2
WEIGHT_TURN_BIG = 1
WEIGHT_BACK = 0
WEIGHT_NONE = 0


bias_rows = bytes( [

WEIGHT_FORWARD, WEIGHT_BACK, WEIGHT_NONE,       # intermediate nodes
WEIGHT_BACK, WEIGHT_FORWARD, WEIGHT_NONE,

WEIGHT_FORWARD, WEIGHT_BACK, WEIGHT_TURN_SMALL, # first turn
WEIGHT_BACK, WEIGHT_FORWARD, WEIGHT_TURN_BIG,
WEIGHT_TURN_BIG, WEIGHT_TURN_SMALL, WEIGHT_BACK,

WEIGHT_FORWARD, WEIGHT_BACK, WEIGHT_TURN_BIG,   # second turn
WEIGHT_BACK, WEIGHT_FORWARD, WEIGHT_TURN_SMALL,
WEIGHT_TURN_SMALL, WEIGHT_TURN_BIG, WEIGHT_BACK,

] )

INTERMEDIATE_FW, INTERMEDIATE_BW, N5_FW, N5_BW, N5_T, N11_FW, N11_BW, N11_T = range(8)
BAD = 0xff

# (5 or 11), 0, 1, 2, 3, 4, [5], 6, 7, 8, 9, 10, [11], 12, 13, 14, 15, 16

turn_table = [ [INTERMEDIATE_FW, INTERMEDIATE_BW, BAD] for _ in range(N_ROWS) ]

for strip in range(0, N_ROWS, N_LEDS_PER_STRIP):

    turn_table[strip+SPLIT_A-1][MOVE_FW] = N5_FW
    turn_table[strip+SPLIT_A+1][MOVE_BW] = N5_BW
    neigh_a = neighbours[strip+SPLIT_A][MOVE_TURN]
    if strip_index(neigh_a) == START:
        turn_table[strip+SPLIT_A][MOVE_TURN] = INTERMEDIATE_FW
        turn_table[neigh_a][MOVE_BW]         = N5_T
    else:
        assert strip_index(neigh_a) == END
        turn_table[strip+SPLIT_A][MOVE_TURN] = INTERMEDIATE_BW
        turn_table[neigh_a][MOVE_FW]         = N5_T


    turn_table[strip+SPLIT_B-1][MOVE_FW] = N11_FW
    turn_table[strip+SPLIT_B+1][MOVE_BW] = N11_BW
    neigh_b = neighbours[strip+SPLIT_B][MOVE_TURN]
    if strip_index(neigh_b) == START:
        turn_table[strip+SPLIT_B][MOVE_TURN] = INTERMEDIATE_FW
        turn_table[neigh_b][MOVE_BW]         = N11_T
    else:
        assert strip_index(neigh_b) == END
        turn_table[strip+SPLIT_B][MOVE_TURN] = INTERMEDIATE_BW
        turn_table[neigh_b][MOVE_FW]         = N11_T

assert len(neighbours) == N_ROWS
for n in neighbours:
    assert len(n) == N_CHOICES_PER_STEP

assert len(turn_table) == N_ROWS
for t in turn_table:
    assert len(t) == N_CHOICES_PER_STEP

out.write( bias_rows )
out.write( array.array("H", (e for row in neighbours for e in row ) ) )
out.write( array.array("H", (e for row in turn_table for e in row ) ) )


