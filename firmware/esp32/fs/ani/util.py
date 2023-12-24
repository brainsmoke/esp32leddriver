
import uarray
import cball

smooth_wave = None
smooth_wave_size = 4096
def get_smooth_wave():
    global smooth_wave
    if not smooth_wave:
        smooth_wave = uarray.array('H', (0 for _ in range(smooth_wave_size)))
        cball.wave_for_gradient_lut(smooth_wave) # same calculation, speed up boot time
    return smooth_wave


