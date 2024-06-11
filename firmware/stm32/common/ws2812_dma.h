#ifndef WS2812_DMA_H
#define WS2812_DMA_H

#include "config.h"

#ifndef DMA_WIDTH
#error "DMA_WIDTH needs to be defined (8 or 16)"
#elif DMA_WIDTH != 16 && DMA_WIDTH != 8
#error "unsupported DMA_WIDTH"
#endif

#define DMA_MASK ((1<<DMA_WIDTH)-1)

#ifndef PIN_MASK
#define PIN_MASK (DMA_MASK)
#endif

#if PIN_MASK & DMA_MASK != PIN_MASK
#error "only pins 0..(" ##DMA_WIDTH "-1) supported"
#endif

#define MAX_STRIPS (DMA_WIDTH)

#include "ws2812_common.h"

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>

#if DMA_WIDTH == 16
typedef uint16_t transfer_t;
#elif DMA_WIDTH == 8
typedef uint8_t transfer_t;
#endif

typedef struct __attribute__((packed,aligned(4)))
{
	/* bitbang bits before carry addition, INVERTED(!) */
	transfer_t bit7;
	transfer_t bit6;
	transfer_t bit5;
	transfer_t bit4;
	transfer_t bit3;
	transfer_t bit2;
	transfer_t bit1;
	transfer_t bit0;

} transposed_t;

/* fail compilation if assumptions made in asm are broken */
_Static_assert( FRAME_SIZE == sizeof(transposed_t)*N_VALUES_PER_STRIP, "FRAME_SIZE bad" );

/* 8 transfers/byte means 1 byte/dma width */
_Static_assert( DMA_WIDTH == sizeof(transposed_t), "size of transposed_t not right" );
_Static_assert( 16 == MAX_STRIPS, "MAX_STRIPS != 16" );

void ws2812_dma_init(GPIO_TypeDef *gpio, transfer_t mask, int t0h, int t1h, int t_pulse);
void ws2812_dma_start(volatile transfer_t buf[], uint32_t transfers);

/* callbacks, implement yourself */
void ws2812_half_transfer(void);
void ws2812_full_transfer(void);

#endif

#endif // WS2812_DMA_H
