
import uarray
import model

import config as _config

class Movie:

    def __init__(self, leds, config=None, filename=None, fps=None, tmp16=None, **kwargs):
        self.file = model.open_file(filename, "rb")
        if fps is None:
            self.in_fps = _config.fps
        else:
            self.in_fps = fps

        self.config_fps = _config.fps

        self.i=self.config_fps

    def next_frame(self, out):

        self.i += self.in_fps
        if self.i < self.config_fps:
            self.file.seek(-len(out)*2, 1)
        else:
            self.i %= self.config_fps

        if self.file.readinto(out) != len(out)*2:
            self.file.seek(0)
            assert self.file.readinto(out) == len(out)*2
        
