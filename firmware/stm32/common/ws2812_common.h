#ifndef WS2812_COMMON_H
#define WS2812_COMMON_H

#include "config.h"

/* derived constants */

#define N_VALUES_PER_STRIP (N_LEDS_PER_STRIP*N_VALUES_PER_LED)
#define N_BITS_PER_STRIP (N_VALUES_PER_STRIP*8)
#define MAX_LEDS (N_LEDS_PER_STRIP*MAX_STRIPS)
#define MAX_VALUES (N_VALUES_PER_STRIP*MAX_STRIPS)

#define FRAME_SIZE (MAX_VALUES)

#if F_CPU == 48000000

#ifndef T0H
#define T0H      (13)
#endif
#ifndef T1H
#define T1H      (31)
#endif
#ifndef T_PULSE
#define T_PULSE  (45)
#endif
#ifndef T_LATCH
#define T_LATCH  (13440)
#endif

#else
#error "timings not specified for CPU speed"
#endif

#define FRAME_CYCLES (T_PULSE*N_BITS_PER_STRIP)
#define SYSTICK_DIV (F_CPU/F_SYS_TICK_CLK)
#define SYSTICK_PERIOD ( (FRAME_CYCLES+T_LATCH+SYSTICK_DIV-1)/SYSTICK_DIV )

#endif // WS2812_COMMON_H
