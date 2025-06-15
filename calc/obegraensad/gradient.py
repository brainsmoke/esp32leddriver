from led_locations import get_leds

from cmath import polar,tau

def scale(arr):
    arr = list(arr)
    min_v = min(arr)
    arr = [ v-min_v for v in arr ]
    max_v = max(arr)
    arr = [ int(65536*v/max_v) for v in arr ]
    return arr

leds=get_leds()


n, m=1, 3
radar = [int((polar(x+y*1j)[1]/tau)*65536)%65536 for x, y, z in leds ]
dist = scale( polar(x+y*1j)[0] for x, y, z in leds )

x_rot = scale( x for x,y,z in leds )
y_rot = scale( y for x,y,z in leds )


v = [ int(a*n+b*m)%65536 for a, b in zip(radar, dist) ]

rot = [ v[i] for i in range(256) ]

def json_array(arr):
    print ("[")
    print(','.join(f"{x:d}" for x in arr))
    print ("]")

def json_array_16x16(arr):
    print ("[")
    for i in range(16):
        c=','
        if i==15:
            c=''
        print(','.join(f"{x:d}" for x in arr[i*16:(i+1)*16])+c)
        
    print ("]")

print ('{')
print (f'"speed": 2')
print (",")
print ('"rotations": ')
json_array_16x16(rot)
print('}')

