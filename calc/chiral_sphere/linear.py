
import math

def dist(a, b):
    return math.sqrt(sum(map(lambda i,j: (i-j)*(i-j), a, b)))

def magnitude( a ):
    x, y = a
    return math.sqrt( x*x + y*y )

def scalar_product( a, b ):
    ax, ay = a
    bx, by = b
    return ax*bx+ay*by

def cross_product(a, b, c):
    """Cross product AB x AC"""
    ax, ay = a
    bx, by = b
    cx, cy = c
    return (bx-ax) * (cy-ay) - (by-ay) * (cx-ax)

def vector_add( a, b ):
    ax, ay = a
    bx, by = b
    return (ax+bx, ay+by)

def vector_sub( a, b ):
    ax, ay = a
    bx, by = b
    return (ax-bx, ay-by)

def scalar_mul( s, a ):
    x, y = a
    return (s*x, s*y)

def normalize( a ):
    x, y = a
    d = math.sqrt(x*x+y*y)
    return (x/d, y/d)

def interpolate( a, b, frac ):
    ax, ay = a
    bx, by = b
    return ( bx*frac+ax*(1-frac), by*frac+ay*(1-frac) )

def segment_intersection( s1, s2 ):
    a, b = s1
    c, d = s2
    cross1 = cross_product(a, b, c)
    cross2 = cross_product(a, d, b)

    cross3 = cross_product(c, d, a)
    cross4 = cross_product(c, b, d)

    interpol = 0
    if cross1+cross2 != 0:
        interpol = cross1/(cross1+cross2)

    if (cross1 == 0 or cross2 == 0 or ( cross1 > 0 ) == ( cross2 > 0)) and \
       (cross3 == 0 or cross4 == 0 or ( cross3 > 0 ) == ( cross4 > 0)) :
        return interpolate(c, d, interpol)

    return None

def scalar_product3( a, b ):
    ax, ay, az = a
    bx, by, bz = b
    return ax*bx+ay*by+az*bz

def vector_add3( a, b ):
    ax, ay, az = a
    bx, by, bz = b
    return (ax+bx, ay+by, az+bz)

def vector_sub3( a, b ):
    ax, ay, az = a
    bx, by, bz = b
    return (ax-bx, ay-by, az-bz)

def scalar_mul3( s, a ):
    x, y, z = a
    return (s*x, s*y, s*z)

def normalize3( a ):
    x, y, z = a
    d = math.sqrt(x*x+y*y+z*z)
    return (x/d, y/d, z/d)

def cross_product3( a, b ):
    (ax,ay,az) = a
    (bx,by,bz) = b
    return ( ay*bz-by*az, az*bx-bz*ax, ax*by-bx*ay )

def identity_matrix():
    return ( (1,0,0),
             (0,1,0),
             (0,0,1) )

def rot_x_matrix(a):
    cos_a, sin_a = math.cos(a), math.sin(a)
    return ( (     1,     0,      0 ),
             (     0, cos_a, -sin_a ),
             (     0, sin_a,  cos_a ) )

def rot_y_matrix(a):
    cos_a, sin_a = math.cos(a), math.sin(a)
    return ( ( cos_a,     0,  sin_a ),
             (     0,     1,      0 ),
             (-sin_a,     0,  cos_a ) )

def rot_z_matrix(a):
    cos_a, sin_a = math.cos(a), math.sin(a)
    return ( ( cos_a, -sin_a,     0 ),
             ( sin_a,  cos_a,     0 ),
             (     0,      0,     1 ) )

def matrix_vector_mul(M, v):
    return tuple([ scalar_product3(M[row], v) for row in range(len(M)) ])

def matrix_mul(Ma, Mb):
    return tuple( tuple( sum(Ma[i][k]*Mb[k][j] for k in range(len(Ma[i])))
                  for j in range(len(Mb[i])))
                  for i in range(len(Ma)) )

def matrix_look_at(points, eye, center=(0,0,0), north=(0,1,0)):
    f     = normalize3(vector_sub3(center, eye))
    f_neg = normalize3(vector_sub3(eye, center))
    up = normalize3(north)
    s = normalize3(cross_product3(f, up))
    u = cross_product3(s, f)
    return ( s, u, f_neg )

