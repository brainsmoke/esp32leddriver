#include <stdlib.h>
#include "stm32f0xx.h"

#include "bitbang16.h"
#include "util.h"

uint16_t frame_a[N_VALUES];
uint16_t frame_b[N_VALUES];
uint16_t * volatile cur;
uint16_t *next;

#define RECV_BUF_SZ (512)
volatile uint8_t recv_buf[RECV_BUF_SZ];

#define O(c) (1<<(2*c))
#define ALT_FN(c) (2<<(2*c))
#define SWD (ALT_FN(13)|ALT_FN(14))

void SysTick_Handler(void)
{
	bitbang16(cur, &GPIOA->ODR);
}

static void init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		remainders[i] = i*153;
	cur = frame_a;
	next = frame_b;
	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD|O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7);

	GPIOB->ODR = 1<<1;
	GPIOB->MODER = O(1);
//	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, 48e6/500000);
	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, 48e6/1e6);
	enable_sys_tick(F_SYS_TICK_CLK/1000);
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

static int read_next_frame(void)
{
	int i=0, end_frame=0, good_frame=1, c;

	i+= 3*3*15;

	for(;;)
	{
		c = dma_getchar();

		if (end_frame && (c == 0xf0))
			return 0; /* end of frame out of sync, re-sync, throw away frame */

		c |= dma_getchar()<<8;

		if (c == 0xffff)
		{
			good_frame = !end_frame;
			end_frame = 1;
			continue;
		}

		if (end_frame)
		{
			if (c == 0xf0ff)
				return good_frame;

			good_frame = 0;
		}

		if (c > 0xff00)
			good_frame = 0;

		if ( (i < N_VALUES) && good_frame )
		{
			next[i] = c;
			i++;
		}
	}
}

int main(void)
{
	init();
	for(;;)
	{
		uint16_t *tmp = cur;
		cur = next; /* swap is not atomic, but only the assignment of cur needs to be */
		next = tmp;
		while (! read_next_frame() );
	}
}

