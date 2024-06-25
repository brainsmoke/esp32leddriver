/*
 * Copyright (c) 2024 Erik Bosman <erik@minemu.org>
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

#include "uartrelay.h"
#include "ws2812_dma.h"
#include "util.h"

/* uart init, slightly different from util.c verision */

volatile uint8_t recv_buf[RECV_BUF_SZ];

static void usart1_rx_dma5_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_SYSCFGEN;

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
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_USART1RX_DMA_RMP;
    DMA1_Channel5->CCR |= DMA_CCR_EN;
}

static void usart1_rx_pa10_dma5_tx_pa9_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale)
{
    GPIOA->MODER |= ALT_FN(10) | ALT_FN(9); /* alternate function mode for PA9/10 */
    GPIOA->AFR[AFR_REG(9)]  |= AFR_SHIFT(9);  /* mux PA9  to usart1_tx */
    GPIOA->AFR[AFR_REG(10)] |= AFR_SHIFT(10); /* mux PA10 to usart1_rx */
    usart1_rx_dma5_enable(buf, size, baudrate_prescale);
	USART1->CR1 |= USART_CR1_TE;
}

static int dma_getchar(void)
{
	static uint32_t last = 0;
	if (last == 0)
		last = RECV_BUF_SZ;
	if (last == DMA1_Channel5->CNDTR)
		return -1;
	last--;
	return recv_buf[RECV_BUF_SZ-1-last];
}


/* / uart init */

frame_t frames[3];
volatile int cur_out=0,cur_in=1;

#include "fsm.h"

int s = GOOD;
uint32_t read_index=0;
uint16_t read_val=0x0000;
uint8_t *in_buf;

void input_init(void)
{
	cur_out = 0;
	cur_in = 1;
	in_buf = (uint8_t *)(frames[cur_in].in);
}

void handle_input(int c)
{
	if (read_index < N_VALUES*2)
		in_buf[read_index] = c;
	else
		USART1->TDR = c;

	read_index++;
	s = fsm[s+fsm_map[c]];

	if (FSM_END(s))
	{
		while (read_index > N_VALUES*2)
			read_index -= N_VALUES*2;

		if ( (s == GOOD_RETURN) && (read_index == 4) )
		{
			cur_out = cur_in;
			cur_in += 1;
			if (cur_in >= 3)
				cur_in = 0;
		}

		s = GOOD;
		read_index = 0;
		in_buf = (uint8_t *)(frames[cur_in].in);
	}
}


void yield(void)
{
	if ( !(USART1->ISR & USART_ISR_TXE) )
		return;

	int c = dma_getchar();
	if ( c == -1 )
		return;

	handle_input(c);
}

uint8_t residual[N_VALUES];
out_t out[N_VALUES_PER_STRIP];


void SysTick_Handler(void)
{
	ws2812_dma_start((transfer_t *)out, N_TRANSFERS);
}

volatile int full=0,half=0;

void ws2812_half_transfer(void)
{
	half=1;
}

void ws2812_full_transfer(void)
{
	full=1;
}

static void ws2812_init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		residual[i] = i*157;

	memset(out, 0x0, sizeof(out));
	memset(frames, 0x0, sizeof(frames));

	input_init();
	ws2812_dma_init(GPIOA, PIN_MASK, T0H, T1H, T_PULSE);
}

static void init(void)
{
	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  // enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->OSPEEDR = 1<<0;
	GPIOA->MODER = O(0)|SWD;

	usart1_rx_pa10_dma5_tx_pa9_enable(recv_buf, RECV_BUF_SZ, 48e6/BAUDRATE);
	ws2812_init();
	NVIC_SetPriority (SysTick_IRQn, 0);
	enable_sys_tick(SYSTICK_PERIOD);
}

void do_dithering_scathering(frame_t *f, int i, int end)
{
	uint32_t a=0x08040201, b=(0x01010101<<OUT_PIN);
	for (; i<end; i++)
	{
		uint32_t v = f->in[i]+residual[i];
		residual[i] = v;
		uint32_t m = ~ ( (v>>8)*a );
		out[i].bits3210 = (m>>3)&b;
		out[i].bits7654 = (m>>7)&b;
		yield();
	}
}

int main(void)
{
	init();
	frame_t *f;
	for (;;)
	{
		while (!half)
			yield();
		half=0;
		f = &frames[cur_out];
		yield();
		do_dithering_scathering(f, 0, N_VALUES/2);
		yield();

		while (!full)
			yield();

		full=0;
		yield();
		do_dithering_scathering(f, N_VALUES/2, N_VALUES);
		yield();
	}
}

