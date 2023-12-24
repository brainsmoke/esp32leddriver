
import uarray
import cball

smooth_wave = None
_smooth_wave_size = 4096
def get_smooth_wave():
    global smooth_wave
    if not smooth_wave:
        smooth_wave = uarray.array('H', (0 for _ in range(_smooth_wave_size)))
        cball.wave_for_gradient_lut(smooth_wave) # same calculation, speed up boot time
    return smooth_wave

sawtooth_wave = None
_sawtooth_wave_size = 4096
def get_sawtooth_wave():
    global sawtooth_wave
    if not sawtooth_wave:
        sawtooth_wave = uarray.array('H', (0 for _ in range(_sawtooth_wave_size)))
        for i in range(_sawtooth_wave_size):
            ix = (5120-i)%4096
            sawtooth_wave[i] = (ix*ix)>>8
    return sawtooth_wave

half_duty_pwm = None
_half_duty_pwm_size = 4096
def get_half_duty_pwm():
    global half_duty_pwm
    if not half_duty_pwm:
        half_duty_pwm = uarray.array('H', (0xffff*int(1024 <= i < 3072) for i in range(_half_duty_pwm_size)))
    return half_duty_pwm


small_duty_pwm = None
_small_duty_pwm_size = 4096
def get_small_duty_pwm():
    global small_duty_pwm
    if not small_duty_pwm:
        small_duty_pwm = uarray.array('H', (0xffff*int(1792 <= i < 2304) for i in range(_small_duty_pwm_size)))
    return small_duty_pwm

_maps_cached=False
inside_map=None
outside_map=None

def _cache_mappings(leds):
    global inside_map, outside_map, _maps_cached
    if True in leds.inside:
        inside_map=uarray.array('H', int(leds.inside[i//3])*0xffff0 for i in range(leds.n_leds*3))
        outside_map=uarray.array('H', int(not leds.inside[i//3])*0xffff0 for i in range(leds.n_leds*3))
    _maps_cached=True

def get_inside_mapping(leds):
    if not _maps_cached:
        _cache_mappings(leds)
    return inside_map

def get_outside_mapping(leds):
    if not _maps_cached:
        _cache_mappings(leds)
    return outside_map

def have_inside(leds):
    return get_inside_mapping(leds) != None
