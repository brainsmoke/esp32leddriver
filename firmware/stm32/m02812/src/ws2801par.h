#ifndef WS2801PAR_H
#define WS2801PAR_H

#include "util.h"

/* configurable parameters (SRAM needed: more than 9*8*N_LEDS_PER_STRIP + 512 bytes) */

/* no bigger than 80x3 or 60x4 */
#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (25)
#endif

/* static constants */

#define N_STRIPS (12) /* code changes needed to change this */

/* derived constants */

#define N_VALUES_PER_STRIP (N_LEDS_PER_STRIP*N_VALUES_PER_LED)
#define N_LEDS (N_LEDS_PER_STRIP*N_STRIPS)
#define N_VALUES (N_VALUES_PER_STRIP*N_STRIPS)

#if N_VALUES_PER_STRIP > 240
#error "strips too long"
#endif

#define CLK_MASK (0x8181)

#define SPEED (2000000)
#define CYCLES_PER_CLK (F_CPU/SPEED)
#define MIN_CYCLES_PER_CLK (12)
#define EXTRA_CYCLES (CYCLES_PER_CLK-MIN_CYCLES_PER_CLK)
#define EXTRA_CYCLES_PRE_CLK (EXTRA_CYCLES>>1)
#define EXTRA_CYCLES_POST_CLK (EXTRA_CYCLES-EXTRA_CYCLES_PRE_CLK)

/* not including preamble/epilogue, just for timeout purposes */
#define BITBANG_CYCLES (CYCLES_PER_CLK*N_VALUES_PER_STRIP*8)

#define FRAME_CYCLES (BITBANG_CYCLES+F_CPU/1000)
#define SYS_TICK_FRAMERATE (FRAME_CYCLES/(F_CPU/F_SYS_TICK_CLK))

#define GPIO_ODR_OFFSET (0x14)
#define GPIO_BSRR_OFFSET (0x18)

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct __attribute__((packed,aligned(4)))
{
	/* carry bits on the fly dithering calculation */
	uint16_t zero;
	uint16_t carry;
	/* bitbang bits before carry addition */
	uint16_t bit1;
	uint16_t bit0;
	uint16_t bit3;
	uint16_t bit2;
	uint16_t bit5;
	uint16_t bit4;
	uint16_t bit7;
	uint16_t bit6;

} transposed_t;

void bitbang_ws2801(transposed_t *frame, uint32_t values_per_strip, uint32_t clkmask, volatile void *gpio_periph);
void precomp_dithering(transposed_t *frame, uint8_t *lowbytes, uint8_t *residual, uint32_t values_per_strip);
void write_value(transposed_t *frame, uint32_t pin, uint32_t c);


#endif

#endif // WS2801PAR_H
