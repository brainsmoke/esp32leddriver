import sys

from linear import normalize3

def coords(c):
    x, y, z = c
    return f"[ {x:g}, {y:g}, {z:g} ]"

def led_desc(pos):
    return '\t\t{\n\t\t\t"position": ' + coords(pos) + \
               ',\n\t\t\t"normal": '   + coords(normalize3(pos)) + \
               ',\n\t\t\t"inside": false\n\t\t}'

def get_leds():
    dx = 20.
    dy = 30.

    w=16
    h=16

    z = 60.

    midx = (w-1)*dx/2
    midy = (h-1)*dy/2

    return [ (j*dx-midx, i*dy-midy, z) for i in range(h) for j in range(w) ]


if __name__ == '__main__':
    leds = get_leds()
    print ('{\n\t"leds": [')
    print(',\n'.join(led_desc(pos) for pos in leds))
    print('\t]\n}')

