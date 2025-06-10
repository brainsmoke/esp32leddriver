#ifndef WS2812_NEW_H
#define WS2812_NEW_H

#include "config.h"

#include "ws2812_dma.h"

#if PIN_MASK != 0xffff
#error "only 16 all pins supported for now (see ws2812_new_asm.S:asm_read_frame)"
#endif

/* offsetoff for asm routines, assertion in C part of header should verify */
#define LOW_BYTES_OFFSET (FRAME_SIZE)

#define RECV_BUF_SZ (4096)

#define DMA_CHANNEL_CNDTR (0x4002005c)

#ifndef __ASSEMBLER__

typedef struct __attribute__((packed,aligned(4)))
{
	uint32_t bit0;
	uint32_t bit1;
	uint32_t bit2;
	uint32_t bit3;
	uint32_t bit4;
	uint32_t bit5;
	uint32_t bit6;
	uint32_t bit7;

} wide_transposed_lsb_t;

typedef struct __attribute__((packed,aligned(4)))
{
	wide_transposed_lsb_t res;
	uint32_t carry;

} residual_t;

typedef struct
{
	transposed_t transpose[N_VALUES_PER_STRIP];
	residual_t low_bytes[N_VALUES_PER_STRIP/2];

} frame_t;

/* fail compilation if assumptions made in asm are broken */
_Static_assert(N_VALUES_PER_STRIP % 4 == 0);
_Static_assert(FRAME_SIZE == sizeof(transposed_t)*N_VALUES_PER_STRIP, "FRAME_SIZE bad");
_Static_assert(LOW_BYTES_OFFSET == offsetof(frame_t, low_bytes), "LOW_BYTES_OFFSET bad");
_Static_assert( (1<<4) == sizeof(transposed_t), "size of transposed_t not 1<<4");
_Static_assert( 16 == MAX_STRIPS, "MAX_STRIPS != 16");

int asm_read_frame(frame_t *f);
void ws2812_asm_apply_dither(frame_t *f, size_t start_byte, size_t end_byte, residual_t residuals[]);

#endif

#endif // WS2812_NEW_H
