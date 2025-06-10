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
#include "fsm.h"

frame_t frame_a, frame_b;

enum
{
	STATE_CONTINUE,
	STATE_BUFFER_READ,
	STATE_SWAPPING,
};

residual_t residual[N_VALUES_PER_STRIP/2];

int volatile bufstate;
frame_t * volatile cur;
frame_t * volatile next;

volatile uint8_t recv_buf[RECV_BUF_SZ];
void SysTick_Handler(void)
{
	ws2812_dma_start((uint16_t *)cur->transpose, N_BITS_PER_STRIP);
}

#define SAFE_HALF_TRANSFER ( (N_VALUES_PER_STRIP)/2 )

void clear_carry(void)
{
	int i;
	for (i=0; i<N_VALUES_PER_STRIP/2; i++)
		residual[i].carry = 0;
}

void ws2812_half_transfer(void)
{
//GPIOA->ODR |= (1<<9);
	if (bufstate == STATE_BUFFER_READ)
	{
		bufstate = STATE_SWAPPING;
		frame_t *tmp = cur;
		cur = next;
		next = tmp;
		clear_carry();
	}
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
	memset(&f->low_bytes, 0x00, sizeof(f->low_bytes));
}

static void ws2812_init(void)
{
	int i;
	uint32_t c = 0, pin;
	memset(residual, 0, sizeof(residual));
	for (pin=1; pin; pin<<=1)
		for (i=0; i<N_VALUES_PER_STRIP/2; i++)
		{
			if (c & 0x01)
				residual[i].res.bit0 |= pin;
			if (c & 0x02)
				residual[i].res.bit1 |= pin;
			if (c & 0x04)
				residual[i].res.bit2 |= pin;
			if (c & 0x08)
				residual[i].res.bit3 |= pin;
			if (c & 0x10)
				residual[i].res.bit4 |= pin;
			if (c & 0x20)
				residual[i].res.bit5 |= pin;
			if (c & 0x40)
				residual[i].res.bit6 |= pin;
			if (c & 0x80)
				residual[i].res.bit7 |= pin;
			c += 159;
		}

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

static int read_next_frame(void)
{
	clear_buf(next);
	int c = asm_read_frame(next), s = GOOD;

	if (c == 0xffff)
		s = BAD_FFFF; /* incomplete frame */
	else if (c > 0xff00)
		s = BAD_FF;

	for(;;)
	{
		s=fsm[s+fsm_map[dma_getchar()]];
		if (FSM_END(s))
			return s == GOOD_RETURN;
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

