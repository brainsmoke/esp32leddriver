#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

/* configurable parameters (SRAM needed: XXX) */

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (88)
#endif

#endif // CONFIG_H
