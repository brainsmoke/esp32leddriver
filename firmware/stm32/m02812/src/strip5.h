#ifndef STRIP5_H
#define STRIP5_H

/* configurable parameters (SRAM needed: more than 9*8*N_LEDS_PER_STRIP + 512 bytes) */

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (102)
#endif

/* static constants */

#define N_STRIPS (5) /* code changes needed to change this */

/* derived constants */

#define N_VALUES_PER_STRIP (N_LEDS_PER_STRIP*N_VALUES_PER_LED)
#define N_LEDS (N_LEDS_PER_STRIP*N_STRIPS)
#define N_VALUES (N_VALUES_PER_STRIP*N_STRIPS)
#define N_BYTES (N_VALUES*2)

#define GPIOA_ODR (0x48000014)

#define WS2812TIMEOUT (F_CPU/1000000*280)
#define FRAME_BYTE_COUNT (N_VALUES_PER_STRIP)  /* count includes skipped pulse */

#define PULSE_WIDTH_MIN (44*8)
#define PULSE_WIDTH_MAX (90*8)
#define PULSE_WIDTH_DEFAULT (45*8)

#define RECV_BUF_SZ (2048)

#ifndef __ASSEMBLER__

#include <stdint.h>
#include "util.h"

extern uint8_t remainders[N_VALUES];
extern uint16_t *cur;

void SysTick_Handler(void);
void strip5_bitbang(uint16_t *buf, volatile uint16_t *gpio_out, uint32_t pulse_width8x);

#endif

#endif //STRIP5_H
