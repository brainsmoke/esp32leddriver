#ifndef BITBANG16_H
#define BITBANG16_H

#include "util.h"

/* configurable parameters (SRAM needed: more than 9*8*N_LEDS_PER_STRIP + 512 bytes) */

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_SEGMENT
#define N_LEDS_PER_SEGMENT (4)
#endif

#ifndef N_SEGMENTS
#define N_SEGMENTS (15)
#endif

#define N_LEDS_PER_STRIP ((N_LEDS_PER_SEGMENT)*(N_SEGMENTS))

/* static constants */

#define N_STRIPS (4) /* code changes needed to change this */

/* derived constants */

#define N_VALUES_PER_SEGMENT (N_LEDS_PER_SEGMENT*N_VALUES_PER_LED)
#define N_VALUES_PER_STRIP (N_LEDS_PER_STRIP*N_VALUES_PER_LED)
#define N_LEDS (N_LEDS_PER_STRIP*N_STRIPS)
#define N_VALUES (N_VALUES_PER_STRIP*N_STRIPS)
#define N_BYTES (N_VALUES*2)

#define ROUTING_TABLE_SIZE (N_STRIPS*(N_SEGMENTS-1))
#define ROUTING_TABLE_SIZE_PADDED (ROUTING_TABLE_SIZE+1)

#if N_VALUES_PER_STRIP > 240
#error "strips too long"
#endif

#define GPIOA_ODR (0x48000014)

#define WS2812TIMEOUT (F_CPU/1000000*280)
#define FRAME_BYTE_COUNT (N_VALUES_PER_STRIP+N_SEGMENTS-1)  /* count includes skipped pulse */

#ifndef __ASSEMBLER__

#include <stdint.h>

extern uint8_t remainders[N_VALUES];
extern uint16_t *cur;

void SysTick_Handler(void);
void bitbang16(uint16_t *buf, volatile uint16_t *gpio_out, uint32_t pulse_width8x);

#endif

#endif // BITBANG16_H
