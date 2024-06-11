#ifndef WS2812_DMA16_H
#define WS2812_DMA16_H

#include "config.h"

#ifndef PIN_MASK
#define PIN_MASK (0xffff)
#endif

#if PIN_MASK & 0xffff != PIN_MASK
#error "only pins 0..15 supported"
#endif

#define MAX_STRIPS (16)

/* derived constants */

#define N_VALUES_PER_STRIP (N_LEDS_PER_STRIP*N_VALUES_PER_LED)
#define N_LEDS (N_LEDS_PER_STRIP*MAX_STRIPS)
#define N_VALUES (N_VALUES_PER_STRIP*MAX_STRIPS)
#define N_BITS_PER_STRIP (N_VALUES_PER_STRIP*8)

#if F_CPU == 48000000

#ifndef T0H
#define T0H      (13)
#endif
#ifndef T1H
#define T1H      (31)
#endif
#ifndef T_PULSE
#define T_PULSE  (44)
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

#define FRAME_SIZE (16*N_VALUES_PER_STRIP)

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>


typedef struct __attribute__((packed,aligned(4)))
{
	/* bitbang bits before carry addition, INVERTED(!) */
	uint16_t bit7;
	uint16_t bit6;
	uint16_t bit5;
	uint16_t bit4;
	uint16_t bit3;
	uint16_t bit2;
	uint16_t bit1;
	uint16_t bit0;

} transposed_t;

/* fail compilation if assumptions made in asm are broken */
_Static_assert(FRAME_SIZE == sizeof(transposed_t)*N_VALUES_PER_STRIP, "FRAME_SIZE bad");
_Static_assert( (1<<4) == sizeof(transposed_t), "size of transposed_t not 1<<4");
_Static_assert( 16 == MAX_STRIPS, "MAX_STRIPS != 16");

void ws2812_dma_init(GPIO_TypeDef *gpio, uint16_t mask, int t0h, int t1h, int t_pulse);
void ws2812_dma_start(volatile uint16_t buf[], uint32_t length);

/* callbacks, implement yourself */
void ws2812_half_transfer(void);
void ws2812_full_transfer(void);

#endif

#endif // WS2812_DMA16_H
