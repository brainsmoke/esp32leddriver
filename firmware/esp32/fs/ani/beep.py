
class Beep:

    def __init__(self, leds, config=None):
        self.phase = 0
        self.count = 0

    def next_frame(self, fbuf):
        self.phase = (self.phase+1)%60
        for i in range(len(fbuf)):
            fbuf[i] = 0

        if self.phase == 0:
            self.count = (self.count+1)%24

#        if self.phase > 30:
        for i in range(24):
            if self.count == i:
                fbuf[i*3] = 2
                fbuf[i*3+1] = 2
                fbuf[i*3+2] = 2

#    def __init__(self, leds, config=None):
#        self.count = list( 0 for _ in range(leds.n_leds) )
#        self.phase = 0

#    def next_frame(self, fbuf):
#        self.phase = (self.phase+1)%60
#        for i in range(len(fbuf)):
#            fbuf[i] = 0

#        if self.phase == 0:
#            for i in range(len(self.count)):
#                self.count[i] = (self.count[i] + 1) % ((i&7)+4)

#        if self.phase > 30:
#            for i in range(len(self.count)):
#                if self.count[i] <= (i&7):
#                    fbuf[i*3] = 2
#                    if i > 7:
#                        fbuf[i*3+1] = 2
#                        if i > 15:
#                            fbuf[i*3+2] = 2

