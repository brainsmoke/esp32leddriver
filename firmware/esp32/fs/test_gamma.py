
import utime, gc, uarray
import cball


def test():
    gamma_map = uarray.array('H', 0 for i in range(256))

    gc.collect()
    cball.calc_gamma_map( gamma_map, 2.2, 0xff00, 0x18 )
    gc.collect()
    cball.calc_gamma_map( gamma_map, 2.2, 0xff00, 0x18 )

    t0 = utime.ticks_us()

    for i in range(1000):
        cball.calc_gamma_map( gamma_map, 2.2, 0xff00, 0x18 )

    t1 = utime.ticks_us()

    print ("cball.calc_gamma_map()", t1-t0 )



    gc.collect()
    cball.calc_gamma_map_fast( gamma_map, 2.2, 0xff00, 0x18 )
    gc.collect()
    cball.calc_gamma_map_fast( gamma_map, 2.2, 0xff00, 0x18 )

    t0 = utime.ticks_us()

    for i in range(1000):
        cball.calc_gamma_map_fast( gamma_map, 2.2, 0xff00, 0x18 )

    t1 = utime.ticks_us()

    print ("cball.calc_gamma_map_fast()", t1-t0 )


    gc.collect()
    cball.calc_gamma_map_sieve( gamma_map, 2.2, 0xff00, 0x18 )
    gc.collect()
    cball.calc_gamma_map_sieve( gamma_map, 2.2, 0xff00, 0x18 )

    t0 = utime.ticks_us()

    for i in range(1000):
        cball.calc_gamma_map_sieve( gamma_map, 2.2, 0xff00, 0x18 )

    t1 = utime.ticks_us()

    print ("cball.calc_gamma_map_sieve()", t1-t0 )

    for max_ in (0xff00,):
        for gamma10 in range(20, 50):
            gamma = gamma10/10.
            cball.calc_gamma_map_sieve( gamma_map, gamma, max_, 0 )
            factor = max_ / 255**gamma
            for i, g_tab in enumerate(gamma_map):
                g = int( factor * i**gamma )
                if abs(g_tab - g ) > 1: 
                    print (gamma, i, hex(g), hex(gamma_map[i]) )

