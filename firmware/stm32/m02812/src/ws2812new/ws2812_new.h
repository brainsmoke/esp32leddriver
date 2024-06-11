#ifndef WS2812_NEW_H
#define WS2812_NEW_H

#include "config.h"
#include "ws2812_dma16.h"

#define PIN_MASK (0xffff)

#if PIN_MASK != 0xffff
#error "only 16 all pins supported for now (see ws2812_new_asm.S:asm_read_frame)"
#endif

/* offsetoff for asm routines, assertion in C part of header should verify */
#define LOW_BYTES_OFFSET (FRAME_SIZE)
#define OLD_CARRY_OFFSET (LOW_BYTES_OFFSET+N_VALUES)

#define RECV_BUF_SZ (4096)

#define DMA_CHANNEL_CNDTR (0x4002005c)

#ifndef __ASSEMBLER__

typedef struct
{
    transposed_t transpose[N_VALUES_PER_STRIP];
    uint8_t low_bytes[N_VALUES];
    uint16_t old_carry[N_VALUES_PER_STRIP];

} frame_t;

/* fail compilation if assumptions made in asm are broken */
_Static_assert(FRAME_SIZE == sizeof(transposed_t)*N_VALUES_PER_STRIP, "FRAME_SIZE bad");
_Static_assert(LOW_BYTES_OFFSET == offsetof(frame_t, low_bytes), "LOW_BYTES_OFFSET bad");
_Static_assert(OLD_CARRY_OFFSET == offsetof(frame_t, old_carry), "OLD_CARRY_OFFSET bad");
_Static_assert( (1<<4) == sizeof(transposed_t), "size of transposed_t not 1<<4");
_Static_assert( 16 == MAX_STRIPS, "MAX_STRIPS != 16");
_Static_assert( OLD_CARRY_OFFSET == LOW_BYTES_OFFSET*2, "weird frame_t layout" );

int asm_read_frame(frame_t *f);
void ws2812_asm_apply_dither(frame_t *f, size_t start_byte, size_t end_byte, uint8_t residuals[]);

#endif

#endif // WS2812_NEW_H
