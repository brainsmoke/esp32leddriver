
import math

class Orbit:

    def __init__(self, leds):
        self.leds = leds
        self.Gmdt2 = -6.674e-11*5.97219e24*1.*1./(6.371e6**3)
        self.objects = (
            ( [1.047088, 0., 0.], [0., 3.943998e-4, 1.274987e-3], (0, 0, 255*0.394)),
            ( [0., 0., 1.470884], [1.134319e-3, 3.149046e-4, 0.], (255*0.394, 255*0.394, 64*0.394)),
            ( [0., 2.569612, 0.], [-4.989389e-4,0., 5.923611e-4], (255*2.463, 32*2.463, 127*2.463))
        )

    def update(self, n):
        for _ in range(n):
            for o in self.objects:
                p, v_dt = o[0], o[1]
                d2 = p[0]*p[0] + p[1]*p[1] + p[2]*p[2]
                d3 = d2 * math.sqrt(d2)
                a = self.Gmdt2/d3
                v_dt[0] += a*p[0]
                v_dt[1] += a*p[1]
                v_dt[2] += a*p[2]
                p[0] += v_dt[0]
                p[1] += v_dt[1]
                p[2] += v_dt[2]

    def next_frame(self, fbuf):
        self.update(10)
        for i in range(self.leds.n_leds):
            r, g, b = 0,0,0
            lpos = self.leds.positions[i]
            normal = self.leds.normals[i]
            for p, _, color in self.objects:
                dpos = ( lpos[0]-p[0], lpos[1]-p[1], lpos[2]-p[2] )
                sprod = dpos[0]*normal[0]+dpos[1]*normal[1]+dpos[2]*normal[2]
                if sprod <= 0:
                    continue
                d2 = dpos[0]*dpos[0] + dpos[1]*dpos[1] + dpos[2]*dpos[2]
                angle = sprod/math.sqrt(d2)
                factor = angle/d2
                r += color[0]*factor
                g += color[1]*factor
                b += color[2]*factor
            ri = int(r)
            gi = int(g)
            bi = int(b)
            fbuf[i*3  ] = min(255, ri)
            fbuf[i*3+1] = min(255, gi)
            fbuf[i*3+2] = min(255, bi)

