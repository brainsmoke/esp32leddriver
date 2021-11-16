#include <stdlib.h>
#include "stm32f0xx.h"

/* re-use the 16 bit protocol from the temporal dithering RGB balls,
 * but don't dither for now.
 */

#include "uartball16.h"
#include "util.h"

#define GOOD          0
#define GOOD_00       1
#define GOOD_01_FE    2
#define GOOD_FF       3
#define GOOD_FFFF     4
#define GOOD_FFFFFF   5
#define BAD           6
#define BAD_FF        7
#define BAD_FFFF      8
#define BAD_FFFFFF    9

#define STATE_COUNT  10

#define GOOD_RETURN  11
#define BAD_RETURN   12

#define IN_00     0
#define IN_01_EF  1
#define IN_F0     2
#define IN_F1_FE  3
#define IN_FF     4

const uint8_t fsm[STATE_COUNT][8] =
{
	[GOOD]        = { [IN_00]=GOOD_00, [IN_01_EF]=GOOD_01_FE, [IN_F0]=GOOD_01_FE , [IN_F1_FE]=GOOD_01_FE, [IN_FF]=GOOD_FF     },
	[GOOD_00]     = { [IN_00]=GOOD   , [IN_01_EF]=GOOD      , [IN_F0]=GOOD       , [IN_F1_FE]=GOOD      , [IN_FF]=GOOD        },
	[GOOD_01_FE]  = { [IN_00]=GOOD   , [IN_01_EF]=GOOD      , [IN_F0]=GOOD       , [IN_F1_FE]=GOOD      , [IN_FF]=BAD_FF      },
	[GOOD_FF]     = { [IN_00]=GOOD   , [IN_01_EF]=GOOD      , [IN_F0]=GOOD       , [IN_F1_FE]=GOOD      , [IN_FF]=GOOD_FFFF   },
	[GOOD_FFFF]   = { [IN_00]=BAD    , [IN_01_EF]=BAD       , [IN_F0]=BAD        , [IN_F1_FE]=BAD       , [IN_FF]=GOOD_FFFFFF },
	[GOOD_FFFFFF] = { [IN_00]=BAD    , [IN_01_EF]=BAD       , [IN_F0]=GOOD_RETURN, [IN_F1_FE]=BAD       , [IN_FF]=BAD_FFFFFF  },
	[BAD]         = { [IN_00]=BAD    , [IN_01_EF]=BAD       , [IN_F0]=BAD        , [IN_F1_FE]=BAD       , [IN_FF]=BAD_FF      },
	[BAD_FF]      = { [IN_00]=BAD    , [IN_01_EF]=BAD       , [IN_F0]=BAD        , [IN_F1_FE]=BAD       , [IN_FF]=BAD_FFFF    },
	[BAD_FFFF]    = { [IN_00]=BAD    , [IN_01_EF]=BAD       , [IN_F0]=BAD        , [IN_F1_FE]=BAD       , [IN_FF]=BAD_FFFFFF  },
	[BAD_FFFFFF]  = { [IN_00]=BAD    , [IN_01_EF]=BAD       , [IN_F0]=BAD_RETURN , [IN_F1_FE]=BAD       , [IN_FF]=BAD_FFFFFF  },

};

uint8_t frame_a[N_BAUDS];
uint8_t frame_b[N_BAUDS];

uint8_t *cur;
uint8_t * volatile next;
volatile uint8_t *recv_p;

#define RECV_BUF_SZ (1024)
volatile uint8_t recv_buf[RECV_BUF_SZ];

#define O(c) (1<<(2*c))
#define ALT_FN(c) (2<<(2*c))
#define SWD (ALT_FN(13)|ALT_FN(14))

volatile uint8_t frame_ready;
int cur_baud;
#define MIN_WAIT_BAUD (70)
void SysTick_Handler(void)
{
	if (cur_baud < N_BAUDS)
	{
		GPIOA->ODR = cur[cur_baud];
		cur_baud++;
	}
	else if ( cur_baud < N_BAUDS+MIN_WAIT_BAUD )
	{
		cur_baud++;
	}
	else if (frame_ready)
	{
		cur_baud = 0;
		uint8_t *tmp = cur;
		cur = next; /* swap is not atomic, but only the assignment of next needs to be */
		next = tmp;
		frame_ready = 0;
	}
}

void init(void)
{
	cur_baud = N_BAUDS+MIN_WAIT_BAUD;
	frame_ready = 0;
	cur = frame_a;
	next = frame_b;
	recv_p=recv_buf;
	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD|O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7);

	GPIOB->ODR = 1<<1;
	GPIOB->MODER = O(1);
	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, 48e6/1e6);
}

static int read_next_frame(void)
{
	int i, j, c;
	uint8_t *buf = next;

	/* start bits / clear data */
	for (i=0; i<N_BAUDS; i++)
		buf[i] = 0;

	/* stop bits */
	for (i=9; i<N_BAUDS; i+=10)
		buf[i] = 0xff;

	for(j=0; j<N_STRIPS; j++)
	{
		for(i=0; i<N_BAUDS; i+=20)
		{
			if (recv_p == &recv_buf[RECV_BUF_SZ])
				recv_p -= RECV_BUF_SZ;

			while(recv_p == &recv_buf[RECV_BUF_SZ-DMA1_Channel3->CNDTR]);

			c = *recv_p++;

			if (recv_p == &recv_buf[RECV_BUF_SZ])
				recv_p -= RECV_BUF_SZ;

			while(recv_p == &recv_buf[RECV_BUF_SZ-DMA1_Channel3->CNDTR]);

			c |= (*recv_p++)<<8;

			if (c > 0xff00)
				break;

			c *= 0x101;

			int bit = 1<<j;

			if ( c & 0x000100 )
				buf[i+1] |= bit;
			if ( c & 0x000200 )
				buf[i+2] |= bit;
			if ( c & 0x000400 )
				buf[i+3] |= bit;
			if ( c & 0x000800 )
				buf[i+4] |= bit;
			if ( c & 0x001000 )
				buf[i+5] |= bit;
			if ( c & 0x002000 )
				buf[i+6] |= bit;
			if ( c & 0x004000 )
				buf[i+7] |= bit;
			if ( c & 0x008000 )
				buf[i+8] |= bit;

			if ( c & 0x010000 )
				buf[i+11] |= bit;
			if ( c & 0x020000 )
				buf[i+12] |= bit;
			if ( c & 0x040000 )
				buf[i+13] |= bit;
			if ( c & 0x080000 )
				buf[i+14] |= bit;
			if ( c & 0x100000 )
				buf[i+15] |= bit;
			if ( c & 0x200000 )
				buf[i+16] |= bit;
			if ( c & 0x400000 )
				buf[i+17] |= bit;
			if ( c & 0x800000 )
				buf[i+18] |= bit;

			c = 0;
		}
		if (c > 0xff00)
			break;
	}

	int s=GOOD;

	if (c == 0xffff)
		s = GOOD_FFFF;
	else if (c > 0xff00)
		s = BAD_FF;

	for(;;)
	{
		if (recv_p == &recv_buf[RECV_BUF_SZ])
			recv_p -= RECV_BUF_SZ;

		while(recv_p == &recv_buf[RECV_BUF_SZ-DMA1_Channel3->CNDTR]);

		c = *recv_p++;

		if (c == 0)
			i = IN_00;
		else if (c < 0xf0)
			i = IN_01_EF;
		else if (c == 0xf0)
			i = IN_F0;
		else if (c == 0xff)
			i = IN_FF;
		else
			i = IN_F1_FE;

		s = fsm[s][i];

		if (s == GOOD_RETURN)
			return 1;
		else if (s == BAD_RETURN)
			return 0;
	}
}

int main(void)
{
	init();
	enable_sys_tick(F_SYS_TICK_CLK/BAUDRATE);
	for(;;)
	{
		while ( frame_ready );
		while (! read_next_frame() );
		frame_ready = 1;
	}
}
