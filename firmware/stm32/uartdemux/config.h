#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

/* configurable parameters (SRAM needed: XXX) */

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (264)
#endif

#define FOREACH_PIN(X) X(0) X(1) X(2) X(3) X(4)
#define STRIP_MASK (0x1f)

#define UART_IN_BAUDRATE (4800000)
#define UART_OUT_BAUDRATE (1500000)

#define DMA_WIDTH (8)

#if (STRIP_MASK & ((1<<DMA_WIDTH)-1)) != STRIP_MASK
#error "current STRIP_MASK not supported with current DMA_WIDTH"
#endif

#endif // CONFIG_H
