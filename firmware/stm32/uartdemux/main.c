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

typedef struct
{
	transfer_t bauds[N_BAUDS];

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

static int dma_getchar(void)
{
	static uint32_t last = 0;
	if (last == 0)
		last = RECV_BUF_SZ;
	while (last == DMA1_Channel3->CNDTR);
	last--;
	return recv_buf[RECV_BUF_SZ-1-last];
}

#include "fsm.h"

int read_next_frame(frame_t *f)
{
	int pin,i,c,s=GOOD;

	for (pin=1; pin<STRIP_MASK; pin<<=1)
		if (pin & STRIP_MASK)
			for (i=0; i<N_BYTES_PER_STRIP*10; i+=10)
			{
				c = dma_getchar();
				if (c & 0x01)
					f->bauds[i+1] |= pin;
				if (c & 0x02)
					f->bauds[i+2] |= pin;
				if (c & 0x04)
					f->bauds[i+3] |= pin;
				if (c & 0x08)
					f->bauds[i+4] |= pin;
				if (c & 0x10)
					f->bauds[i+5] |= pin;
				if (c & 0x20)
					f->bauds[i+6] |= pin;
				if (c & 0x40)
					f->bauds[i+7] |= pin;
				if (c & 0x80)
					f->bauds[i+8] |= pin;

				s=fsm[s+fsm_map[c]];
				if (FSM_END(s))
					return 0;
			}

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

