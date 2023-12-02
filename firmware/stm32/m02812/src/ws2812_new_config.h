#ifndef WS2812_NEW_CONFIG_H
#define WS2812_NEW_CONFIG_H

#include "util.h"

/* configurable parameters (SRAM needed: more than 9*8*N_LEDS_PER_STRIP + 512 bytes) */

/* no bigger than 80x3 or 60x4 */
#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (80)
#endif

#endif // WS2812_NEW_CONFIG_H
