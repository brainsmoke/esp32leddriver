
import uarray
import cball

_wave_size = 4096

smooth_wave = None
def get_smooth_wave():
    global smooth_wave
    if not smooth_wave:
        smooth_wave = uarray.array('H', (0 for _ in range(_wave_size)))
        cball.wave_for_gradient_lut(smooth_wave) # same calculation, speed up boot time
    return smooth_wave

sawtooth_wave = None
def get_sawtooth_wave():
    global sawtooth_wave
    if not sawtooth_wave:
        sawtooth_wave = uarray.array('H', (0 for _ in range(_wave_size)))
        for i in range(_wave_size):
            ix = (1024+i)%4096
            sawtooth_wave[i] = (ix*ix)>>8
    return sawtooth_wave

half_duty_pwm = None
def get_half_duty_pwm():
    global half_duty_pwm
    if not half_duty_pwm:
        half_duty_pwm = uarray.array('H', (0xffff*int(1024 <= i < 3072) for i in range(_wave_size)))
    return half_duty_pwm


small_duty_pwm = None
def get_small_duty_pwm():
    global small_duty_pwm
    if not small_duty_pwm:
        small_duty_pwm = uarray.array('H', (0xffff*int(1792 <= i < 2304) for i in range(_wave_size)))
    return small_duty_pwm


_blur_size = 128
sawtooth_wave_blurred = None
def get_sawtooth_wave_blurred():
    global sawtooth_wave_blurred
    if not sawtooth_wave_blurred:
        sawtooth_wave_blurred = uarray.array('H', (0 for _ in range(_wave_size)))
        for i in range(_wave_size):
            sawtooth_wave_blurred[i-1024] = (i*i)>>8
        for i in range(-_blur_size, 0):
            v = (sawtooth_wave_blurred[i-1024]*-i)//_blur_size
            sawtooth_wave_blurred[i-1024] = v
    return sawtooth_wave_blurred

half_duty_pwm_blurred = None
def get_half_duty_pwm_blurred():
    global half_duty_pwm_blurred
    if not half_duty_pwm_blurred:
        half_duty_pwm_blurred = uarray.array('H', (0xffff*int(1024 <= i < 3072) for i in range(_wave_size)))
        mid = _blur_size >> 1
        for i in range(_blur_size-1):
            half_duty_pwm_blurred[1024-mid+i] = 0xffff*(i+1)//_blur_size
            half_duty_pwm_blurred[3072-mid+i] = 0xffff*(_blur_size-i-1)//_blur_size

    return half_duty_pwm_blurred


small_duty_pwm_blurred = None
def get_small_duty_pwm_blurred():
    global small_duty_pwm_blurred
    if not small_duty_pwm_blurred:
        small_duty_pwm_blurred = uarray.array('H', (0xffff*int(1792 <= i < 2304) for i in range(_wave_size)))
        mid = _blur_size >> 1
        for i in range(_blur_size-1):
            small_duty_pwm_blurred[1792-mid+i] = 0xffff*(i+1)//_blur_size
            small_duty_pwm_blurred[2304-mid+i] = 0xffff*(_blur_size-i-1)//_blur_size
    return small_duty_pwm_blurred

_maps_cached=False
inside_map=None
outside_map=None

def _cache_mappings(leds):
    global inside_map, outside_map, _maps_cached
    if True in leds.inside and False in leds.inside:
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

def has_sides(leds):
    return get_inside_mapping(leds) != None
