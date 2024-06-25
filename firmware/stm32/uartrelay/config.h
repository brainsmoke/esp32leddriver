#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

/* configurable parameters (SRAM needed: XXX) */

#define T_PULSE (47)

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (44)
#endif

#define DMA_WIDTH (8)

#define OUT_PIN (0)

#define BAUDRATE (1500000)

#endif // CONFIG_H
