import sys, math

f_in=sys.stdin.buffer
f_out=sys.stdout.buffer

gamma, brightness = 1, 1

try:
    gamma = float(argv[3])
    brightness = float(argv[4])
except:
    pass

class Dim:
    def __init__(self, x, y):
        self.x, self.y = x, y

    def __str__(self):
        return f'({self.x}, {self.y}) '

num_leds = Dim( 16, 16 )
pitch =  Dim( 20, 30 )
led_size = Dim (12,12)

dim_out = Dim( num_leds.x*pitch.x, num_leds.y*pitch.y )

dim_in = Dim( int(sys.argv[1]), int(sys.argv[2]) )
frame_size = dim_in.x*dim_in.y


if dim_in.x * dim_out.y > dim_in.y * dim_out.x:
    crop = Dim( dim_out.x * dim_in.y/dim_out.y, dim_in.y )
else:
    crop = Dim( dim_in.x, dim_out.y * dim_in.x/dim_out.x )

crop_off = Dim( (dim_in.x-crop.x)/2, (dim_in.y-crop.y)/2 )

led_d = led_size.x*crop.x/(num_leds.x*pitch.x)


def get_columns():
    cols = []

    led_r = led_d/2
    in_pitch = crop.x/num_leds.x
    for x_out in range(num_leds.x):
        x_mid = crop_off.x + (x_out+.5)*in_pitch
        x_start = math.ceil(x_mid-led_r)
        x_end = math.floor(x_mid+led_r)
        for x_in in range(x_start, x_end+1):
            cols.append((x_in,x_out))

    return cols

def get_rows():
    rows = []

    led_r = led_d/2
    in_pitch = crop.y/num_leds.y
    for y_out in range(num_leds.y):
        y_mid = crop_off.y + (y_out+.5)*in_pitch
        y_start = math.ceil(y_mid-led_r)
        y_end = math.floor(y_mid+led_r)
        for y_in in range(y_start, y_end+1):
            rows.append((y_in,y_out))

    return rows

cols = get_columns()
rows = get_rows()

def process_frame(data):

    accum = [0]*(num_leds.x*num_leds.y)
    weight = [0]*(num_leds.x*num_leds.y)

    for in_y, out_y in rows:
        for in_x, out_x in cols:
            accum[out_y*num_leds.x+out_x] += data[in_y*dim_in.x+in_x]
            weight[out_y*num_leds.x+out_x] += 255

    arr = [ int(0xff00*(a+.5)//w) for a, w in zip(accum, weight) ]
    barr = bytearray(2*len(arr))
    for i in range(len(arr)):
        barr[i*2] = arr[i]&0xff
        barr[i*2+1] = arr[i]>>8

    return barr

data = f_in.read(frame_size)
while len(data) == frame_size:

    f_out.write(process_frame(data))
    f_out.flush()
    data = f_in.read(frame_size)

assert len(data) == 0 
