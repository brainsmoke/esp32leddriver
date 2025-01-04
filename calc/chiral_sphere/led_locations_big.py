
# python led_locations_big.py 39.69147828724952 > leds_real_size.json

import math

import sphere_tilings
from linear import *

A, alpha = sphere_tilings.chiral_2_to_1_angles(kind='penta')


step_x = matrix_mul( rot_x_matrix(-A), rot_z_matrix(-alpha/2) )
step_y = matrix_mul( rot_z_matrix(3*alpha/2), rot_x_matrix(math.pi) )

strip_bend = 3*alpha/2

leds_per_strip = 44
leds_in_line = leds_per_strip + 2


def penta():
    m = identity_matrix()
    for i in range(5):
        yield m
        m = matrix_mul(m, step_y)
        m = matrix_mul(m, step_x)
        m = matrix_mul(m, step_y)

def extend():
    m = identity_matrix()
    yield m
    m = matrix_mul(m, step_x)
    yield m
    m = matrix_mul(m, step_x)
    yield m
    m = matrix_mul(m, step_x)
    m = matrix_mul(m, step_y)
    yield m
    m = matrix_mul(m, step_x)
    yield m
    m = matrix_mul(m, step_x)
    yield m

def strips():
    for Ma in penta():
        for Mb in extend():
            yield matrix_mul(Ma, Mb)

def leds():
    for Ma in strips():
        for arc_i in range(1, leds_per_strip+1):
            arc = arc_i * strip_bend/(leds_per_strip+1)
            Mrot = rot_z_matrix(arc)
            yield matrix_mul(Ma, Mrot)

def print_m(m):
    a,b,c=m[0]
    d,e,f=m[1]
    g,h,i=m[2]
    print (f"""multmatrix (m=[[{a},{b},{c},0],[{d},{e},{f},0],[{g},{h},{i},0],[0,0,0,1]])children();""")

def print_scad_test(led_normals):
    print ("module leds()\n{\n")
    for pos in led_normals:
        x,y,z = pos
        print (f"translate([{x},{y},{z}])children();")
    print("}\n""")
    print("scale([10,10,10])leds()sphere(.02);\n""")

def print_scad_test2():
    print ("module leds_rotations()\n{\n")
    for m in leds():
        print_m(m)
    print("}\n""")
    print("leds_rotations()translate([10,0,0])sphere(.2);");

led_normals = [ matrix_vector_mul(m, (1,0,0)) for m in leds() ]
top = vector_add3( led_normals[(leds_per_strip-1)//2], led_normals[leds_per_strip//2] )
Mrot = rot_z_matrix(math.pi/2)
eye = matrix_vector_mul(Mrot, top)
orient = matrix_look_at(led_normals, eye, north = top);
led_normals = [ matrix_vector_mul(orient, v) for v in led_normals ]

import sys

r = float(sys.argv[1])

def coords(c):
    x, y, z = c
    return f"[ {x:f}, {y:f}, {z:f} ]"

def led_desc(normal, r):
    return '\t\t{\n\t\t\t"position": ' + coords(scalar_mul3(r, normal)) + \
               ',\n\t\t\t"normal": '   + coords(normal) + \
               ',\n\t\t\t"inside": false\n\t\t}'

print ('{\n\t"leds": [')
print(',\n'.join(led_desc(pos, r) for pos in led_normals))
print('\t]\n}')

