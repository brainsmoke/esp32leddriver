import sys

rings = [

]

for i in range(6):
	rings.appendfor i in range(6):



remap = [ ]
for ring in rings:
    for segment in ring:
        remap.extend(get_led_segment(*segment))

invmap = [ False ] * len(remap)

for i, v in enumerate(remap):
    invmap[v] = i

print(remap)
print(invmap)


sys.exit(0)

for y in range(0, 42*20, 42):
    print("   ", end='')
    for x in range(42):
        print ('{:4d},'.format(invmap[von[x+y]]), end='')
    print()

