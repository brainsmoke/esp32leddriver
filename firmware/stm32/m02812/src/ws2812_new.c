#include <stdlib.h>
#include <string.h>
#include "stm32f0xx.h"

#include "ws2812_new.h"
#include "util.h"

frame_t frame_a, frame_b;

enum
{
	STATE_CONTINUE,
	STATE_BUFFER_READ,
	STATE_SWAPPING,
};

uint8_t residual[N_VALUES];

int volatile bufstate;
frame_t * volatile cur;
frame_t * volatile next;

#define RECV_BUF_SZ (2048)
volatile uint8_t recv_buf[RECV_BUF_SZ];
void SysTick_Handler(void)
{
	ws2812_dma_start((uint16_t *)cur->transpose, N_VALUES_PER_STRIP);
}

#define SAFE_HALF_TRANSFER ( (N_VALUES_PER_STRIP+1)/2 )

void ws2812_half_transfer(void)
{
	if (bufstate == STATE_BUFFER_READ)
	{
		bufstate = STATE_SWAPPING;
		frame_t *tmp = cur;
		cur = next;
		next = tmp;
	}
	ws2812_asm_apply_dither(cur, 0, SAFE_HALF_TRANSFER, residual);
}

void ws2812_full_transfer(void)
{
	if (bufstate == STATE_SWAPPING)
		bufstate = STATE_CONTINUE;

	ws2812_asm_apply_dither(cur, SAFE_HALF_TRANSFER, N_VALUES_PER_STRIP, residual);
}

static void clear_buf(frame_t *f)
{
	/* bits are inverted, since we use BRR */
	memset(&f->transpose, 0xff, sizeof(f->transpose));

	memset(&f->old_carry, 0x00, sizeof(f->old_carry));
	memset(&f->low_bytes, 0x00, sizeof(f->low_bytes));
}

static void init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		residual[i] = i*157;

	cur = &frame_a;
	next = &frame_b;
	clear_buf(cur);

	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD;

	GPIOB->ODR = 0;
	GPIOB->MODER = O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7)|O(9)|O(9)|O(10)|O(11)|O(12)|O(13)|O(14)|O(15);

	usart2_rx_pa3_dma5_enable(recv_buf, RECV_BUF_SZ, 48e6/6e6);
	ws2812_dma_init(GPIOB, PIN_MASK, T0H, T1H, T_PULSE);
	enable_sys_tick(SYSTICK_PERIOD);
}

static volatile uint8_t *recv_p=recv_buf, *recv_end=recv_buf;

static void dma_wait(void)
{
	if (recv_p == &recv_buf[RECV_BUF_SZ])
		recv_p = recv_end = &recv_buf[0];

	while(recv_p == recv_end)
	{
		recv_end = &recv_buf[RECV_BUF_SZ-DMA1_Channel5->CNDTR];
		if (recv_p > recv_end)
			recv_end = &recv_buf[RECV_BUF_SZ];
	}	
}

enum
{
	GOOD,
	GOOD_00,
	GOOD_01_FE,
	GOOD_FF,
	GOOD_FFFF,
	GOOD_FFFFFF,
	BAD,
	BAD_FF,
	BAD_FFFF,
	BAD_FFFFFF,

	STATE_COUNT,

	GOOD_RETURN,
	BAD_RETURN,
};

enum
{
	IN_00,
	IN_01_EF,
	IN_F0,
	IN_F1_FE,
	IN_FF,
};

static const uint8_t fsm[STATE_COUNT][8] =
{
	[GOOD]        = {  [IN_00] = GOOD_00, [IN_01_EF] = GOOD_01_FE, [IN_F0] = GOOD_01_FE , [IN_F1_FE] = GOOD_01_FE, [IN_FF] = GOOD_FF      , },
	[GOOD_00]     = {  [IN_00] = GOOD   , [IN_01_EF] = GOOD      , [IN_F0] = GOOD       , [IN_F1_FE] = GOOD      , [IN_FF] = GOOD         , },
	[GOOD_01_FE]  = {  [IN_00] = GOOD   , [IN_01_EF] = GOOD      , [IN_F0] = GOOD       , [IN_F1_FE] = GOOD      , [IN_FF] = BAD_FF       , },
	[GOOD_FF]     = {  [IN_00] = GOOD   , [IN_01_EF] = GOOD      , [IN_F0] = GOOD       , [IN_F1_FE] = GOOD      , [IN_FF] = GOOD_FFFF    , },
	[GOOD_FFFF]   = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = GOOD_FFFFFF  , },
	[GOOD_FFFFFF] = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = GOOD_RETURN, [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFFFF   , },
	[BAD]         = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FF       , },
	[BAD_FF]      = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFF     , },
	[BAD_FFFF]    = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFFFF   , },
	[BAD_FFFFFF]  = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD_RETURN , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFFFF   , },

};

static int read_next_frame(void)
{
	int i, p, c;

	clear_buf(next);

	for(p=0; p<16; p++)
	for(i=0; i<N_VALUES_PER_STRIP; i++)
	{
		if (recv_p == recv_end)
			dma_wait();

		c = *recv_p++;
		next->low_bytes[i*16] = c;

		if (recv_p == recv_end)
			dma_wait();

		c |= (*recv_p++)<<8;

		if (c > 0xff00)
			break;

		c >>= 8;

		transposed_t *t = &next->transpose[i];
		int pin = 1<<p;
		/* bits are inverted, since we use BRR */
		if (c & 0x01)
			t->bit0 &=~ pin;
		if (c & 0x02)
			t->bit1 &=~ pin;
		if (c & 0x04)
			t->bit2 &=~ pin;
		if (c & 0x08)
			t->bit3 &=~ pin;
		if (c & 0x10)
			t->bit4 &=~ pin;
		if (c & 0x20)
			t->bit5 &=~ pin;
		if (c & 0x40)
			t->bit6 &=~ pin;
		if (c & 0x80)
			t->bit7 &=~ pin;
	}

	int s=GOOD;

	if (c == 0xffff)
		s = GOOD_FFFF;
	else if (c > 0xff00)
		s = BAD_FF;

	for(;;)
	{
		if (recv_p == recv_end)
			dma_wait();

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
	for(;;)
	{
		while ( bufstate != STATE_CONTINUE );
		while ( ! read_next_frame() );
		bufstate = STATE_BUFFER_READ;
	}
}

