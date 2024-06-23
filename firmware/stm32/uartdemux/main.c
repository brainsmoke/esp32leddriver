/*
 * Copyright (c) 2023-2024 Erik Bosman <erik@minemu.org>
 *
 * Permission  is  hereby  granted,  free  of  charge,  to  any  person
 * obtaining  a copy  of  this  software  and  associated documentation
 * files (the "Software"),  to deal in the Software without restriction,
 * including  without  limitation  the  rights  to  use,  copy,  modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the
 * Software,  and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The  above  copyright  notice  and this  permission  notice  shall be
 * included  in  all  copies  or  substantial portions  of the Software.
 *
 * THE SOFTWARE  IS  PROVIDED  "AS IS", WITHOUT WARRANTY  OF ANY KIND,
 * EXPRESS OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR  PURPOSE  AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN
 * CONNECTION  WITH THE SOFTWARE  OR THE USE  OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * (http://opensource.org/licenses/mit-license.html)
 *
 */

#include <stdlib.h>
#include <string.h>
#include "stm32f0xx.h"

#include "config.h"

#include "util.h"
#include "gpio_dma.h"
#include "fsm.h"

#define N_VALUES_PER_STRIP ((N_VALUES_PER_LED)*(N_LEDS_PER_STRIP))
#define N_BYTES_PER_STRIP (2*N_VALUES_PER_STRIP)

#define UART_OUT_BAUD_CYCLES (F_CPU/BAUDRATE)

#define END_MARKER_SIZE (4)

#define N_BAUDS ( ( N_BYTES_PER_STRIP + END_MARKER_SIZE ) * 10 )

#define RECV_BUF_SZ (256)

typedef struct __attribute__((packed,aligned(4)))
{
	transfer_t _unused0; /* padding to allow for optimisations using aligned loads/stores  */
	transfer_t bauds[N_BAUDS];
	transfer_t _unused1;

} frame_t;

frame_t frame_a, frame_b;

volatile uint8_t recv_buf[RECV_BUF_SZ];

void gpio_half_transfer(void)
{
	/* */
}

volatile int busy;
void gpio_full_transfer(void)
{
	busy = 0;
}

static const uint8_t end_marker[] = { 0xff, 0xff, 0xff, 0xf0 };
_Static_assert(sizeof(end_marker) == END_MARKER_SIZE, "bad END_MARKER_SIZE");

static void init_buf(frame_t *f)
{
	int i, j;
	for (i=0; i<N_BAUDS; i++)
		f->bauds[i] = 0; /* start bit + clear data */

	/* ...  .........................__.
	 *   |  :  :  :  :  :  :  :  :  :  |
	 *   | S: 0: 1: 2: 3: 4: 5: 6: 7: E|
	 *   |__:..:..:..:..:..:..:..:..:  |...
	 */

	for (i=0; i<N_BAUDS; i+=10)
		f->bauds[i+9] = STRIP_MASK; /* stop bits */

	for (i=0; i<sizeof(end_marker); i++)
		for (j=0; j<8; j++)
			if ( end_marker[i] & (1<<j) ) /* little endian */
				f->bauds[ N_BYTES_PER_STRIP*10 + i*10 + 1 + j ] = STRIP_MASK;
}

static void uart_out_init(void)
{
	init_buf(&frame_a);
	init_buf(&frame_b);
	busy=0;
	gpio_dma_init(GPIOB, F_CPU/UART_OUT_BAUDRATE);
	_Static_assert( F_CPU % UART_OUT_BAUDRATE == 0, "baudrate not exact" );
}

static void init(void)
{
	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD;

	GPIOB->OSPEEDR = 0x00000155;
	GPIOB->ODR = STRIP_MASK | 0xff00;
	GPIOB->MODER = O_MASK(STRIP_MASK) | O_MASK(0xff00);

	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, F_CPU/UART_IN_BAUDRATE);
	_Static_assert( F_CPU % UART_IN_BAUDRATE == 0, "baudrate not exact" );
	uart_out_init();
}

static uint32_t dma_cur = 0, dma_end = 0;
static uint8_t dma_getchar(void)
{

	if (dma_cur < dma_end)
		return recv_buf[dma_cur++];

	if (dma_cur == RECV_BUF_SZ)
		dma_cur = 0;

	for (;;)
	{
		dma_end = RECV_BUF_SZ - DMA1_Channel3->CNDTR;

		if (dma_end < dma_cur)
			dma_end = RECV_BUF_SZ;

		if (dma_cur < dma_end)
			return recv_buf[dma_cur++];
	}
}

#include "fsm.h"

static const uint32_t bit_lookup[] =
{
#define BIT_ROW(pin) 0x00000000<<pin, 0x00000001<<pin, 0x00000100<<pin, 0x00000101<<pin, \
                     0x00010000<<pin, 0x00010001<<pin, 0x00010100<<pin, 0x00010101<<pin, \
                     0x01000000<<pin, 0x01000001<<pin, 0x01000100<<pin, 0x01000101<<pin, \
                     0x01010000<<pin, 0x01010001<<pin, 0x01010100<<pin, 0x01010101<<pin,
	FOREACH_PIN(BIT_ROW)
#undef BIT_ROW
	0xff
};

int read_next_frame(frame_t *f)
{
	int s=GOOD;
	uint8_t c;

_Static_assert( (N_BYTES_PER_STRIP & 1) == 0, "");

	const uint32_t *lookup = bit_lookup;
	/*
	 * Bit-scatter the bits for the first output pin into the DMA buffer & clear the old
	 * data at the same time.
	 *
	 * The first byte writes the stop bit (0) to all the outputs
	 * The next byte writes all least significant bits, followed by the 2nd
	 * least significant bit etc.
	 * The tenth byte writes the stop bit (1) to all outputs.
	 *
	 * Speed up bit scattering by using word & half-word memory loads and a lookup table
	 * instead of byte loads & if statements.
	 *
	 * Unroll the loop
	 *
	 */
	for(uint32_t *p = (uint32_t *)f; p<(uint32_t *)&f->bauds[N_BYTES_PER_STRIP*10-1]; p+=5)
	{
		if (dma_cur < dma_end)
			c = recv_buf[dma_cur++];
		else
			c = dma_getchar();

	/*  0  1  2  3| 4  5  6  7| 8  9  10 
	 * ...  .........................__.
	 *   |  :  :  :  :  :  :  :  :  :  |
	 *   | S: 0: 1: 2: 3: 4: 5: 6: 7: E|
	 *   |__:..:..:..:..:..:..:..:..:  |...
	 *       \___/ \_________/ \___/
	 *         A        B        C
	 */

		((uint16_t *)p)[1] = lookup[c&15]; /* A */
		p[1] = lookup[(c>>2)&15];          /* B */
		((uint16_t *)p)[4] = lookup[c>>6]; /* C */

		s=fsm[s+fsm_map[c]];
		if (FSM_END(s))
			return 0;

		if (dma_cur < dma_end)
			c = recv_buf[dma_cur++];
		else
			c = dma_getchar();

	/* 10 11|12 13 14 15|16 17 18 19| 20 21 ..
	 * ...  .........................__.
	 *   |  :  :  :  :  :  :  :  :  :  |
	 *   | S: 0: 1: 2: 3: 4: 5: 6: 7: E|
	 *   |__:..:..:..:..:..:..:..:..:  |...
	 *       \_________/ \_________/
	 *            D          `E
	 */

		p[3] = lookup[c&15]; /* D */
		p[4] = lookup[c>>4]; /* E */

		s=fsm[s+fsm_map[c]];
		if (FSM_END(s))
			return 0;

	}

	/*
	 * Do the same for all the other output pins, but OR the result as not to clear
	 * the previously read strips.
	 *
	 * Every pin gets its own lookup table eliminating the need for extra shifts.
	 *
	 */
	for (lookup+=16; lookup[0]==0; lookup+=16)
		for(uint32_t *p = (uint32_t *)f; p<(uint32_t *)&f->bauds[N_BYTES_PER_STRIP*10-1]; p+=5)
		{
			if (dma_cur < dma_end)
				c = recv_buf[dma_cur++];
			else
				c = dma_getchar();

			((uint16_t *)p)[1] |= lookup[c&15];
			p[1] |= lookup[(c>>2)&15];
			((uint16_t *)p)[4] |= lookup[c>>6];

			s=fsm[s+fsm_map[c]];
			if (FSM_END(s))
				return 0;

			if (dma_cur < dma_end)
				c = recv_buf[dma_cur++];
			else
				c = dma_getchar();

			p[3] |= lookup[c&15];
			p[4] |= lookup[c>>4];

			s=fsm[s+fsm_map[c]];
			if (FSM_END(s))
				return 0;
		}

	int i;
	for (i=0; i<3; i++)
	{
		s=fsm[s+fsm_map[dma_getchar()]];
		if (FSM_END(s))
			return 0;
	}

	for(;;)
	{
		s=fsm[s+fsm_map[dma_getchar()]];
		if (FSM_END(s))
			return s == GOOD_RETURN;
	}
}

void demux_uart(frame_t *f)
{
	while ( !read_next_frame(f) || busy );
	busy = 1;
	gpio_dma_start((transfer_t *)f->bauds, N_BAUDS);
}

int main(void)
{
	init();
	for(;;)
	{
		demux_uart(&frame_a);
		demux_uart(&frame_b);
	}
}

