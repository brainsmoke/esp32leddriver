/*
 * Copyright (c) 2023 Erik Bosman <erik@minemu.org>
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

#include "ws2812_dma16.h"
#include "util.h"

frame_t frame_a, frame_b;

enum
{
	STATE_CONTINUE,
	STATE_BUFFER_READ,
	STATE_SWAPPING,
};

uint8_t residual[MAX_VALUES];

int volatile bufstate;
frame_t * volatile cur;
frame_t * volatile next;

volatile uint8_t recv_buf[RECV_BUF_SZ];
void SysTick_Handler(void)
{
	ws2812_dma_start((uint16_t *)cur->transpose, N_BITS_PER_STRIP);
}

#define SAFE_HALF_TRANSFER ( (N_VALUES_PER_STRIP)/2 )

void ws2812_half_transfer(void)
{
	if (bufstate == STATE_BUFFER_READ)
	{
		bufstate = STATE_SWAPPING;
		frame_t *tmp = cur;
		cur = next;
		next = tmp;
	}
//GPIOA->ODR |= (1<<9);
	ws2812_asm_apply_dither(cur, 0, SAFE_HALF_TRANSFER, residual);
//GPIOA->ODR &=~ (1<<9);
}

void ws2812_full_transfer(void)
{
	if (bufstate == STATE_SWAPPING)
		bufstate = STATE_CONTINUE;

//GPIOA->ODR |= (1<<9);
	ws2812_asm_apply_dither(cur, SAFE_HALF_TRANSFER, N_VALUES_PER_STRIP, residual);
//GPIOA->ODR &=~ (1<<9);
}

static void clear_buf(frame_t *f)
{
	/* bits are inverted, since we use BRR */
	memset(&f->transpose, 0xff, sizeof(f->transpose));

	memset(&f->old_carry, 0x00, sizeof(f->old_carry));
	memset(&f->low_bytes, 0x00, sizeof(f->low_bytes));
}

static void ws2812_init(void)
{
	int i;
	for (i=0; i<MAX_VALUES; i++)
		residual[i] = i*157;

	bufstate = STATE_CONTINUE;
	cur = &frame_a;
	next = &frame_b;
	clear_buf(cur);
	clear_buf(next);

	ws2812_asm_apply_dither(cur, 0, N_VALUES_PER_STRIP, residual);
	ws2812_dma_init(GPIOB, PIN_MASK, T0H, T1H, T_PULSE);
}

void usart1_rx_dma5_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    DMA1_Channel5->CPAR = (uint32_t)&USART1->RDR;
    DMA1_Channel5->CMAR = (uint32_t)buf;
    DMA1_Channel5->CNDTR = size;
    DMA1_Channel5->CCR = DMA_CCR_MINC | DMA_CCR_CIRC | (0*DMA_CCR_MSIZE_0) | (0*DMA_CCR_PSIZE_0);

    if (baudrate_prescale < 0x10)
    {
        USART1->CR1 = USART_CR1_OVER8;
        USART1->BRR = baudrate_prescale+(baudrate_prescale&~7);
    }
    else
    {
        USART1->CR1 = 0;
        USART1->BRR = baudrate_prescale;
    }

    USART1->CR3 = USART_CR3_DMAR;
    USART1->CR1 |= USART_CR1_RE | USART_CR1_UE;
    /* enable dma on usart1_rx */
	#define PSEL *(uint32_t *)(DMA1_BASE+0xa8)
	PSEL |= (8<<16);
    DMA1_Channel5->CCR |= DMA_CCR_EN;
}

void usart1_rx_pa10_dma5_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale)
{
    GPIOA->MODER |= ALT_FN(10); /* alternate function mode for PA10 */
    GPIOA->AFR[AFR_REG(10)] |= AFR_SHIFT(10); /* mux PA10 to usart1_rx */
    usart1_rx_dma5_enable(buf, size, baudrate_prescale);
}


static void init(void)
{
	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD;// | O(9);

    GPIOB->OSPEEDR = 0xaaaaaaaa;
	GPIOB->ODR = 0;
	GPIOB->MODER = O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7)|O(8)|O(9)|O(10)|O(11)|O(12)|O(13)|O(14)|O(15);

	usart1_rx_pa10_dma5_enable(recv_buf, RECV_BUF_SZ, 48e6/48e5);
	ws2812_init();
	NVIC_SetPriority (SysTick_IRQn, 0);
	enable_sys_tick(SYSTICK_PERIOD);
}

volatile uint8_t * dma_in_p = &recv_buf[0];
_Static_assert(DMA_CHANNEL_CNDTR == (uint32_t)&DMA1_Channel5->CNDTR, "DMA channel register define is wrong");
int dma_getchar(void);

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
	int i, c=0;

	clear_buf(next);
	c = asm_read_frame(next);
	int s=GOOD;

	if (c == 0xffff)
		s = GOOD_FFFF;
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

