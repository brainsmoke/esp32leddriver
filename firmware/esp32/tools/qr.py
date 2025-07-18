#!/usr/bin/python3

TOP, RIGHT, BOTTOM, LEFT = 1,2,4,8

next = { TOP:RIGHT, RIGHT:BOTTOM, BOTTOM:LEFT, LEFT:TOP }
previous = { TOP:LEFT, LEFT:BOTTOM, BOTTOM:RIGHT, RIGHT:TOP }
move = { TOP:(1,0), RIGHT:(0,1), BOTTOM:(-1,0), LEFT:(0,-1) }

def has_edge(borders, x, y, dir):
    if dir & (RIGHT|BOTTOM):
        x-=1;
    if dir & (BOTTOM|LEFT):
        y-=1;

    if 0 <= x < len(borders[0]) and 0 <= y < len(borders):
        return borders[y][x] & dir;
    else:
        return 0;

def clear_edge(borders, x, y, dir):
    if dir & (RIGHT|BOTTOM):
        x-=1;
    if dir & (BOTTOM|LEFT):
        y-=1;
    if 0 <= x < len(borders[0]) and 0 <= y < len(borders):
        borders[y][x] &=~ dir

def print_borders(borders):
    for row in borders:
        print(''.join(f"{c:x}" for c in row))
    print()

# creates polygon from connected region and erases it from the borders array
def get_region(borders, x, y, dir):

    path = []

    start = (x,y);
    path.append( (x,y) )

    while True:
        if not has_edge(borders, x, y, dir):
            path.append( (x,y) )

            if has_edge(borders, x, y, next[dir]):
                dir = next[dir];
            else:
                dir = previous[dir];

        clear_edge(borders, x, y, dir);
        dx, dy = move[dir]
        x += dx
        y += dy

        if (x,y) == start:
            break

    return path

def get_regions(borders):
    paths = []
    for y in range(len(borders)):
        for x in range(len(borders[y])):
            if has_edge(borders, x, y, TOP):
                paths.append(get_region(borders, x, y, TOP))

    return paths

def get_borders(bitmap):

    w, h = len(bitmap[0]), len(bitmap)

    def border_bits(x, y):
        if bitmap[y][x] == 0:
            return 0

        bits = 0
        if y-1 < 0 or not bitmap[y-1][x]:
            bits |= TOP
        if y+1 >= h or not bitmap[y+1][x]:
            bits |= BOTTOM
        if x-1 < 0 or not bitmap[y][x-1]:
            bits |= LEFT
        if x+1 >= w or not bitmap[y][x+1]:
            bits |= RIGHT
        return bits

    return [ [ border_bits(x, y) for x in range(len(bitmap[y])) ] for y in range(len(bitmap)) ]

def bitmap(s):

    from shutil import which
    from subprocess import Popen, PIPE

    qrencode= which('qrencode')
    assert qrencode

    with Popen([qrencode, '-t', 'ascii', '--', s], stdout=PIPE) as p:
        qr = [ [ b' #'.index(c) for c in line[::2] if c in b' #' ] for line in p.stdout ]

    for qline in qr:
        assert len(qline) == len(qr)

    return qr

def vector(s):
    """ -> [ list of paths ], width """
    qr = bitmap(s)
    borders = get_borders(qr)
    w, h = len(qr[0]), len(qr)
    paths = get_regions(borders)
    return paths, w

