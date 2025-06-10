
/* Joining the FPGA cult next project, I promise */

#include <stdlib.h>
#include <string.h>
#include "stm32f0xx.h"

#include "obegraensad.h"
#include "util.h"

typedef struct __attribute__((packed,aligned(4)))
{
	uint8_t bit[16][N_BITS_PER_CHANNEL]; 
} frame_t;

frame_t frame_a, frame_b;

static frame_t * volatile cur_frame;
static frame_t * volatile next_frame;
static frame_t * volatile draw_frame;

static uint16_t iter;

#define RECV_BUF_SZ (128)
static volatile uint8_t recv_buf[RECV_BUF_SZ];

enum
{
	B15, B14, B13, B12, B11, B10, B9, B8,
	B7, B6, B5, B4, B3, B2, B1, B0,
	E0, E1, E2, E3, E4, E5, E6, E7, E8,
	ZZZ,
};

/* SysTick dispatch table for BCM bitbang */
static const uint8_t dtable[] =
{
/*	 15    .    .    .    .    .    .    .   14    .    .    .   13    .   12  11/10 dith */

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B8, E8,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B4, E4,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B6, E6,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,  B9,  B2, E2,

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B7, E7,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B3, E3,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B5, E5,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,       B1, E2,

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B8, E8,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B4, E4,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B6, E6,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,  B9,  B2, E2,

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B7, E7,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B3, E3,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B5, E5,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,       B0, E1,

};

#define TABLE_SIZE (sizeof(dtable)/sizeof(dtable[0]))

static void bitbang_15(void)       { bitbang64_clk_stm32(cur_frame->bit[15], (void *)GPIOA); draw_frame = cur_frame; }
static void bitbang_14(void)       { bitbang64_clk_stm32(draw_frame->bit[14], (void *)GPIOA); }
static void bitbang_13(void)       { bitbang64_clk_stm32(draw_frame->bit[13], (void *)GPIOA); }
static void bitbang_12(void)       { bitbang64_clk_stm32(draw_frame->bit[12], (void *)GPIOA); }
static void bitbang_11(void)       { bitbang64_clk_stm32(draw_frame->bit[11], (void *)GPIOA); }
static void bitbang_10(void)       { bitbang64_clk_stm32(draw_frame->bit[10], (void *)GPIOA); }
static void bitbang_9(void)        { bitbang64_clk_stm32(draw_frame->bit[ 9], (void *)GPIOA); }
static void bitbang_8(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[8],  (void *)GPIOA); }
static void bitbang_7(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[7],  (void *)GPIOA); }
static void bitbang_6(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[6],  (void *)GPIOA); }
static void bitbang_5(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[5],  (void *)GPIOA); }
static void bitbang_4(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[4],  (void *)GPIOA); }
static void bitbang_3(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[3],  (void *)GPIOA); }
static void bitbang_2(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[2],  (void *)GPIOA); }
static void bitbang_1(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[1],  (void *)GPIOA); }
static void bitbang_0(void) { bitbang64_clk_no_enable_stm32(draw_frame->bit[0],  (void *)GPIOA); }

//#define FLIP_OFF (SET(BIT_NOT_OUTPUT_ENABLE)|CLEAR(BIT_ENABLE_HIGH))
#define FLIP_OFF (SET(BIT_NOT_OUTPUT_ENABLE))
#define FLIP_ON  (CLEAR(BIT_NOT_OUTPUT_ENABLE))
#define SYSTICK_PERIOD ((uint32_t)(F_SYS_TICK_CLK/(TABLE_SIZE*200) ))
#define SYSTICK_CYCLES ((uint32_t)(8*SYSTICK_PERIOD))

static void enable_8(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>1); }
static void enable_7(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>2); }
static void enable_6(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>3); }
static void enable_5(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>4); }
static void enable_4(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>5); }
static void enable_3(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>6); }
static void enable_2(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>7); }
static void enable_1(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>8); }
static void enable_0(void) { write_wait_write(&GPIOA->BSRR, FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>9); }

static void ret(void) { }

typedef void (*func_t)(void);

const func_t dispatch[] =
{

	[B15] = bitbang_15,
	[B14] = bitbang_14,
	[B13] = bitbang_13,
	[B12] = bitbang_12,
	[B11] = bitbang_11,
	[B10] = bitbang_10,
	[B9] = bitbang_9,
	[B8] = bitbang_8,
	[B7] = bitbang_7,
	[B6] = bitbang_6,
	[B5] = bitbang_5,
	[B4] = bitbang_4,
	[B3] = bitbang_3,
	[B2] = bitbang_2,
	[B1] = bitbang_1,
	[B0] = bitbang_0,

	[E8] = enable_8,
	[E7] = enable_7,
	[E6] = enable_6,
	[E5] = enable_5,
	[E4] = enable_4,
	[E3] = enable_3,
	[E2] = enable_2,
	[E1] = enable_1,
	[E0] = enable_0,

//	[OFF] = off,
	[ZZZ]= ret,
};

void SysTick_Handler(void)
{
	dispatch[dtable[iter]]();

	if (iter < TABLE_SIZE-1)
		iter = iter+1;
	else
		iter = 0;
}

static void init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; /* enable clock on GPIO A & B */
	GPIOA->ODR = BIT_NOT_OUTPUT_ENABLE;
	GPIOA->MODER = SWD|O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7);

	GPIOA->OSPEEDR = OSPEED_HIGH(13)                    |
                     OSPEED_HIGH(PIN_LATCH)             |
	                 OSPEED_HIGH(PIN_DATA_0)            |
	                 OSPEED_HIGH(PIN_DATA_1)            |
	                 OSPEED_HIGH(PIN_DATA_2)            |
	                 OSPEED_HIGH(PIN_DATA_3)            |
	                 OSPEED_HIGH(PIN_CLK)               |
	                 OSPEED_HIGH(PIN_NOT_OUTPUT_ENABLE) ;

	clock48mhz();
	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, 48e6/1e6);

	cur_frame = &frame_a;
	next_frame = &frame_b;
	iter = 0;

	memset(&frame_a, 0, sizeof(frame_a));
	memset(&frame_b, 0, sizeof(frame_b));

	enable_sys_tick(SYSTICK_PERIOD);
}

static inline int dma_getchar(void)
{
	static uint32_t last = 0;
	if (last == 0)
		last = RECV_BUF_SZ;
	while (last == DMA1_Channel3->CNDTR);
	last--;
	return recv_buf[RECV_BUF_SZ-1-last];
}

static inline int dma_get_u16(void)
{
	int c;
	c  =   dma_getchar();
	c |= ( dma_getchar() << 8 );
	return c;
}

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

static const uint8_t module_map[N_BITS_PER_CHANNEL] =
{

	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,

};

static const uint8_t channel[] = { BIT_DATA_0, BIT_DATA_1, BIT_DATA_2, BIT_DATA_3 };

static int finish_frame(int c)
{
	int s=GOOD, i;

	if (c == 0xffff)
		s = BAD_FFFF;
	else if (c > 0xff00)
		s = BAD_FF;

	for(;;)
	{
		c = dma_getchar();

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

static int read_next_frame(void)
{
	int i, j, c;

	memset(next_frame, 0, sizeof(*next_frame));
	memset(next_frame->bit[15], BIT_NOT_OUTPUT_ENABLE, sizeof(next_frame->bit[15]));

	for (j=0; j<N_CHANNELS; j++)
	{
		int bit = channel[j];
		for (i=0; i<N_BITS_PER_CHANNEL; i++)
		{
			c = dma_get_u16();
			if (c > 0xff00)
				return finish_frame(c);

			uint8_t *p = &next_frame->bit[0][module_map[i]];
			for ( ; c ; c >>= 1 )
			{
	            if (c&1)
					*p |= bit;

				p += sizeof(next_frame->bit[0])/sizeof(*p);
			}
		}
	}

	return finish_frame(0);
}

int main(void)
{
	init();

	for(;;)
	{
		while (! read_next_frame() );
		frame_t *tmp = cur_frame;
		cur_frame = next_frame;
		next_frame = tmp;
		while (draw_frame == next_frame);
	}

	return 0;
}

