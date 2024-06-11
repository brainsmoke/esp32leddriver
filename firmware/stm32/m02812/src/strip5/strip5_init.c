#include <stdlib.h>
#include "stm32f0xx.h"

#include "strip5.h"
#include "util.h"
#include "fsm.h"

uint16_t frame_a[N_VALUES];
uint16_t frame_b[N_VALUES];
uint16_t *cur;
uint16_t pulse_width8x;

volatile uint8_t recv_buf[RECV_BUF_SZ];

void init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		remainders[i] = i*153;

	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIO_LEDDATA->MODER |= O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7);

	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, 48e6/2e6);
}

