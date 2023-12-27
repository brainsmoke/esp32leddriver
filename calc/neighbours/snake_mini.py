
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

import sys, array, json

N_CHOICES_PER_STEP = 4
N_BIAS_ROWS = 4 # 1 for each direction for each side
N_ROWS = 120
neighbours = json.load(open(sys.argv[1], "r"))['neighbours']

out = open(sys.argv[2], "wb")

header = b'\1RANDOMWALK' + bytes( (N_CHOICES_PER_STEP, N_BIAS_ROWS&0xff, N_BIAS_ROWS>>8, N_ROWS&0xff, N_ROWS>>8) )

out.write(header)

WEIGHT_SAME = 4
WEIGHT_TURN = 1
WEIGHT_BACK = 0

weights = [WEIGHT_SAME, WEIGHT_TURN, WEIGHT_BACK, WEIGHT_TURN]

bias_rows = []

for i in range(len(weights)):
    bias_rows.extend( weights[-i:] + weights[:-i] )

bias_rows = bytes( bias_rows )

TOP_LEFT, BOTTOM_LEFT, BOTTOM_RIGHT, TOP_RIGHT = range(4)

def opposite(x):
    return (BOTTOM_RIGHT, TOP_RIGHT, TOP_LEFT, BOTTOM_LEFT)[x]

def same_facet(i, j):
    return i//5 == j//5

turn_table = [ [None]*4 for _ in range(24*5) ]

for i, n_list in enumerate(neighbours):
    for d, neighbour in enumerate(n_list):
        if same_facet(i, neighbour):
            turn_table[i][d] = d
        else:
            for opp_d, opp_n in enumerate(neighbours[neighbour]):
                if opp_n == i:
                    turn_table[i][d] = opposite(opp_d)

# symmetry sanity check
for i in range(24):
    assert turn_table[:5] == turn_table[i*5:(i+1)*5]

assert len(neighbours) == N_ROWS
for n in neighbours:
    assert len(n) == N_CHOICES_PER_STEP

assert len(turn_table) == N_ROWS
for t in turn_table:
    assert len(t) == N_CHOICES_PER_STEP


out.write( bias_rows )
out.write( array.array("H", (e for row in neighbours for e in row ) ) )
out.write( array.array("H", (e for row in turn_table for e in row ) ) )

