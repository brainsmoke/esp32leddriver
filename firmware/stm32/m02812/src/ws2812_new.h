#ifndef WS2812_NEW_H
#define WS2812_NEW_H

#include "util.h"

/* configurable parameters (SRAM needed: more than 9*8*N_LEDS_PER_STRIP + 512 bytes) */

/* no bigger than 80x3 or 60x4 */
#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (80)
#endif

/* static constants */

#define N_STRIPS (16) /* code changes needed to change this */
#define PIN_MASK (0xffff)

/* derived constants */

#define N_VALUES_PER_STRIP (N_LEDS_PER_STRIP*N_VALUES_PER_LED)
#define N_LEDS (N_LEDS_PER_STRIP*N_STRIPS)
#define N_VALUES (N_VALUES_PER_STRIP*N_STRIPS)
#define N_BITS_PER_STRIP (N_VALUES_PER_STRIP*8)

#if N_VALUES_PER_STRIP > 240
#error "strips too long"
#endif

#if F_CPU == 48000000

#define T0H           (13)
#define T1H           (31)
#define T_PULSE       (44)
#define T_LATCH       (13440)

#else
#error "timings not specified for CPU speed"
#endif

#define FRAME_CYCLES (T_PULSE*8*N_VALUES_PER_STRIP)
#define SYSTICK_DIV (F_CPU/F_SYS_TICK_CLK)
#define SYSTICK_PERIOD ( (FRAME_CYCLES+SYSTICK_DIV-1)/SYSTICK_DIV )

/* offsetoff for asm routines, assertion in C part of header should verify */
#define LOW_BYTES_OFFSET (16*N_VALUES_PER_STRIP)
#define OLD_CARRY_OFFSET (LOW_BYTES_OFFSET+N_VALUES)

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>


typedef struct __attribute__((packed,aligned(4)))
{
	/* bitbang bits before carry addition, INVERTED(!) */
	uint16_t bit0;
	uint16_t bit1;
	uint16_t bit2;
	uint16_t bit3;
	uint16_t bit4;
	uint16_t bit5;
	uint16_t bit6;
	uint16_t bit7;

} transposed_t;

typedef struct
{
    transposed_t transpose[N_VALUES_PER_STRIP];
    uint8_t low_bytes[N_VALUES];
    uint16_t old_carry[N_VALUES_PER_STRIP];

} frame_t;

/* fail compilation if assumptions made in asm are broken */
_Static_assert(LOW_BYTES_OFFSET == offsetof(frame_t, low_bytes), "LOW_BYTES_OFFSET bad");
_Static_assert(OLD_CARRY_OFFSET == offsetof(frame_t, old_carry), "OLD_CARRY_OFFSET bad");
_Static_assert( (1<<4) == sizeof(transposed_t), "size of transposed_t not 1<<4");
_Static_assert( 16 == N_STRIPS, "N_STRIPS != 16");
_Static_assert( OLD_CARRY_OFFSET == LOW_BYTES_OFFSET*2, "weird frame_t layout" );

void ws2812_dma_start(volatile uint16_t *buf, uint32_t length);
void ws2812_half_transfer(void);
void ws2812_full_transfer(void);
void ws2812_dma_init(GPIO_TypeDef *gpio, uint16_t mask, int t0h, int t1h, int t_pulse);

void ws2812_asm_apply_dither(frame_t *f, size_t start_byte, size_t end_byte, uint8_t residuals[]);

#endif

#endif // WS2812_NEW_H
