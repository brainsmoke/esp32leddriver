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
#include "stm32f0xx.h"

#include "gpio_dma.h"

/* each timer channel/compare register is mapped onto one DMA channel by hardware  */

#define DMA_CHANNEL_DATA  (DMA1_Channel2)

#define TIMER_COMPARE_DATA  (TIM1->CCR1)
#define TIMER_PULSELEN      (TIM1->ARR)

	                                   /* v------- 16 bit ------v */
#define DMA_CONFIG_MEM_TO_PERIPH_16BIT ( (1*DMA_CCR_MSIZE_0) | (1*DMA_CCR_PSIZE_0) | \
                                         (1*DMA_CCR_DIR) | (2*DMA_CCR_PL_0) )

#define DMA_CONFIG_BUF_TO_PERIPH_16BIT ( DMA_CCR_MINC | DMA_CONFIG_MEM_TO_PERIPH_16BIT )

	                                  /* v------- 8 bit -------v */
#define DMA_CONFIG_MEM_TO_PERIPH_8BIT ( (0*DMA_CCR_MSIZE_0) | (0*DMA_CCR_PSIZE_0) | \
                                        (1*DMA_CCR_DIR) | (2*DMA_CCR_PL_0) )

#define DMA_CONFIG_BUF_TO_PERIPH_8BIT ( DMA_CCR_MINC | DMA_CONFIG_MEM_TO_PERIPH_8BIT )

#if DMA_WIDTH == 8
#define DMA_CONFIG_MEM_TO_PERIPH DMA_CONFIG_MEM_TO_PERIPH_8BIT
#define DMA_CONFIG_BUF_TO_PERIPH DMA_CONFIG_BUF_TO_PERIPH_8BIT
#elif DMA_WIDTH == 16
#define DMA_CONFIG_MEM_TO_PERIPH DMA_CONFIG_MEM_TO_PERIPH_16BIT
#define DMA_CONFIG_BUF_TO_PERIPH DMA_CONFIG_BUF_TO_PERIPH_16BIT
#endif

#define DMA_CONFIG_INTERRUPT_HALF_TRANSFER (DMA_CCR_HTIE)
#define DMA_CONFIG_INTERRUPT_FULL_TRANSFER (DMA_CCR_TCIE)

static void gpio_dma_setup(GPIO_TypeDef *gpio)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA_CHANNEL_DATA->CPAR = (uint32_t)&gpio->ODR;
	//DMA_CHANNEL_DATA->CMAR = ...
	DMA_CHANNEL_DATA->CCR  = DMA_CONFIG_BUF_TO_PERIPH |
	                         DMA_CONFIG_INTERRUPT_HALF_TRANSFER |
	                         DMA_CONFIG_INTERRUPT_FULL_TRANSFER ;

	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
	NVIC_SetPriority (DMA1_Channel2_3_IRQn, 1);
}

void gpio_dma_start(volatile transfer_t buf[], uint32_t n_transfers)
{
	DMA_CHANNEL_DATA->CMAR = (uint32_t)buf;
	DMA_CHANNEL_DATA->CNDTR = n_transfers;
	DMA_CHANNEL_DATA->CCR |= DMA_CCR_EN;

	/* start timer */
	TIM1->CR1 |= TIM_CR1_CEN;
}

static void gpio_dma_stop(void)
{
	/* stop timer */
	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM1->DIER &=~ ( TIM_DIER_CC1DE );

	/* disable dma */
	DMA_CHANNEL_DATA->CCR &=~ DMA_CCR_EN;

	TIM1->DIER |= ( TIM_DIER_CC1DE );

	TIM1->CNT = 0;
	TIM1->SR  = 0;
}

void DMA1_Channel2_3_IRQHandler(void)
{
	if (DMA1->ISR & DMA_ISR_TCIF2)
	{
		gpio_dma_stop();
		__enable_irq();
		gpio_full_transfer();
		DMA1->IFCR = DMA_ISR_TCIF2;
	}
	if (DMA1->ISR & DMA_ISR_HTIF2)
	{
		__enable_irq();
		gpio_half_transfer();
		DMA1->IFCR = DMA_ISR_HTIF2;
	}
}

static void gpio_timer_setup(int rate)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	TIM1->CR1 = (0*TIM_CR1_CKD_0) | /* no clock division */
	            (0*TIM_CR1_CMS_0) |
	            (0*TIM_CR1_DIR)   |
	            (0*TIM_CR1_OPM)   |
	            (0*TIM_CR1_URS)   |
	            (0*TIM_CR1_UDIS)  |
	            (0*TIM_CR1_CEN)   ; /* don't start counting yet */

//	TIM1->PSC = 0;
	TIM1->DIER = TIM_DIER_CC1DE;

	/* default */
//	TIM1->CCMR1 = (0*TIM_CCMR1_CC1S_0) ; /* channel 1 = output compare */

	TIMER_PULSELEN      = rate - 1;
	TIMER_COMPARE_DATA  = rate - 1;
}


void gpio_dma_init(GPIO_TypeDef *gpio, int rate)
{
	gpio_dma_setup(gpio);
	gpio_timer_setup(rate);
}

