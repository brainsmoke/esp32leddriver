
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

w = int(sys.argv[1])
h = int(sys.argv[2])
avg_traversal = int(sys.argv[3])

BORDER_BIAS=4

neighbours = [ [ (x+1)%w+y*w, x+((y-1)%h)*w, (x-1)%w+y*w, x+((y+1)%h)*w ] for y in range(h) for x in range(w) ]

assert 1 < BORDER_BIAS < 255
assert 1 < avg_traversal*BORDER_BIAS < 255
assert 1 < avg_traversal*2 < 255

bias = [ avg_traversal*2, 1, 0, 1 ]
bias_border_left = [ avg_traversal*BORDER_BIAS, 1, 0, BORDER_BIAS-1 ]
bias_border_fw = [ avg_traversal*2, BORDER_BIAS, 0, BORDER_BIAS ]
bias_border_right = [ avg_traversal*BORDER_BIAS, BORDER_BIAS-1, 0, 1 ]
bias_border_back = bias

bias_border = [ bias_border_left, bias_border_fw, bias_border_right, bias_border_back ]

bias_corner_left_back = bias_border_left
bias_corner_fw_left = [ avg_traversal, 1, 0, BORDER_BIAS-1 ]
bias_corner_right_fw = [ avg_traversal, BORDER_BIAS-1, 0, 1 ]
bias_corner_back_right = bias_border_right

bias_corner = [ bias_corner_left_back, bias_corner_fw_left, bias_corner_right_fw, bias_corner_back_right ]

bias_rows = []

def rotate(arr, i):
    return arr[-i:] + arr[:-i]

for r in range(4):
    bias_rows.extend(rotate(bias, r))

for i in range(4):
    for r in range(4):
        bias_rows.extend(rotate(bias_border[(i+r)%4], r) )

for i in range(4):
    for r in range(4):
        bias_rows.extend(rotate(bias_corner[(i+r)%4], r) )

orient = [ 0 ] * w*h

TYPE_INTERNAL = 0
TYPE_BORDER = 4
TYPE_CORNER = 20

orient[0] = TYPE_CORNER + 0
orient[w*(h-1)] = TYPE_CORNER + 4
orient[w*h-1] = TYPE_CORNER + 8
orient[w-1] = TYPE_CORNER + 12
for x in range(1, w-1):
    orient[x] = TYPE_BORDER
    orient[w*h-1-x] = TYPE_BORDER+8

for y in range(1, h-1):
    orient[w*h-1-y*h] = TYPE_BORDER+4
    orient[y*h] = TYPE_BORDER+12

#for y in range(h):
#    print ( ' '.join(f'{orient[x+y*w]:2d}' for x in range(w)) )

#for i in range(4):
#    for y in range(h):
#        print ( ' '.join(f'{neighbours[x+y*w][i]:2d}' for x in range(w)) )
#    print()

bias_transitions = [ [ i + orient[neighbours[ix][i]] for i in range(4) ] for ix in range(w*h) ]

N_CHOICES_PER_STEP = 4
N_BIAS_ROWS = len(bias_rows)//N_CHOICES_PER_STEP
N_ROWS = w*h

out = open(sys.argv[4], "wb")

header = b'\1RANDOMWALK' + bytes( (N_CHOICES_PER_STEP, N_BIAS_ROWS&0xff, N_BIAS_ROWS>>8, N_ROWS&0xff, N_ROWS>>8) )

out.write(header)
bias_rows = bytes( bias_rows )
out.write( bias_rows )
out.write( array.array("H", (e for row in neighbours for e in row ) ) )
out.write( array.array("H", (e for row in bias_transitions for e in row ) ) )

