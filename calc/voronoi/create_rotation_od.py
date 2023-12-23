import sys, json, array

from PIL import Image

json = json.load(open(sys.argv[1], "r"))
image = Image.open(sys.argv[2])
outfile = open(sys.argv[3], 'wb')

w, h = json['width'], json['height']
mapping = json['voronoi']
weighting = json['weighting']

n_leds = max(mapping)+1

data = image.getdata()

print(w, h, n_leds)

buf = array.array('H', [0]*n_leds*3)
mv = memoryview(buf)

totals = [0.]*n_leds
for i,v in enumerate(data):
    totals[mapping[i]] += weighting[i//w]

for off in range(w):

    sums = [0.]*(n_leds*3)

    for i,v in enumerate(data):
        x, y = (i+off)%w, i//w
        ix = y*w+x
        sums[mapping[ix]*3  ] += weighting[i//w]*v[0]
        sums[mapping[ix]*3+1] += weighting[i//w]*v[1]
        sums[mapping[ix]*3+2] += weighting[i//w]*v[2]

    for i in range(len(buf)):
        v = int(0x101*sums[i]/totals[i//3])
        if v < 0:
            v = 0
        elif v > 0xffff:
            v = 0xffff
#no inside leds
        if (i//3)%5==4:
            v = 0
        buf[i] = v
    outfile.write(mv)
    print(off)

for i,v in enumerate(data):
    print(i, v)
