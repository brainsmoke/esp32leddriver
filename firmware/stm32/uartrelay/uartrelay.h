#ifndef UARTRELAY_H
#define UARTRELAY_H

#include "config.h"

#define PIN_MASK (1<<(OUT_PIN))
_Static_assert( OUT_PIN < 8, "bad OUTPIN" );
_Static_assert( OUT_PIN >= 0, "bad OUTPIN" );

#include "ws2812_dma.h"

#define N_VALUES (1*N_VALUES_PER_STRIP)

#define RECV_BUF_SZ (32)

#include <stdint.h>
#include <stddef.h>

typedef struct __attribute__((packed,aligned(4)))
{
	/* bitbang bits before carry addition, INVERTED(!) */
	uint32_t bits7654;
	uint32_t bits3210;

} out_t;

typedef struct
{
	uint16_t in[1*N_VALUES_PER_STRIP]; /* we drive only one strip */
} frame_t;

#endif // UARTRELAY_H
