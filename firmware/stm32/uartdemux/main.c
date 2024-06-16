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
	transfer_t _unused0;
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

    GPIOB->OSPEEDR = 0;
	GPIOB->ODR = STRIP_MASK;
	GPIOB->MODER = O_MASK(STRIP_MASK);

	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, F_CPU/UART_IN_BAUDRATE);
	_Static_assert( F_CPU % UART_IN_BAUDRATE == 0, "baudrate not exact" );
	uart_out_init();
}

static uint8_t dma_getchar(void)
{
	static uint32_t cur = 0, end = 0;

	if (cur < end)
		return recv_buf[cur++];

	if (cur == RECV_BUF_SZ)
		cur = 0;

	for (;;)
	{
		end = RECV_BUF_SZ - DMA1_Channel3->CNDTR;
		if (cur < end)
			return recv_buf[cur++];
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
	for(uint32_t *p = (uint32_t *)&f; p<(uint32_t *)&f->bauds[N_BAUDS-1]; p+=5)
	{
		c = dma_getchar();
		((uint16_t *)p)[1] = lookup[c&15];
		p[1] = lookup[(c>>2)&15];
		((uint16_t *)p)[4] = lookup[c>>6];

		s=fsm[s+fsm_map[c]];
		if (FSM_END(s))
			return 0;

		c = dma_getchar();
		p[3] = lookup[c&15];
		p[4] = lookup[c>>4];

		s=fsm[s+fsm_map[c]];
		if (FSM_END(s))
			return 0;

	}

	for (lookup+=16; lookup[0]==0; lookup+=16)
		for(uint32_t *p = (uint32_t *)&f; p<(uint32_t *)&f->bauds[N_BAUDS-1]; p+=5)
		{
			c = dma_getchar();
			((uint16_t *)p)[1] |= lookup[c&15];
			p[1] |= lookup[(c>>2)&15];
			((uint16_t *)p)[4] |= lookup[c>>6];

			s=fsm[s+fsm_map[c]];
			if (FSM_END(s))
				return 0;

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

