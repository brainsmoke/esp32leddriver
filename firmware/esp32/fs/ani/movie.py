
import uarray
import model

class BaseMovie:
    def next_frame(self, out):
        if self.file.readinto(out) != len(out)*2:
            self.file.seek(0)
            assert self.file.readinto(out) == len(out)*2

class Earth(BaseMovie):
    def __init__(self, leds, config=None, **kwargs):
        self.file = model.open_file("earth.bin", "rb")

        
