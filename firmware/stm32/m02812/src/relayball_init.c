#include <stdlib.h>
#include "stm32f0xx.h"

#include "bitbang16_relay_4strip.h"
#include "relayball_init.h"
#include "default_route.h"
#include "util.h"
#include "fsm.h"

uint16_t frame_a[N_VALUES];
uint16_t frame_b[N_VALUES];
uint8_t table[ROUTING_TABLE_SIZE_PADDED];
uint16_t *cur;
uint8_t *routing_table;
uint16_t pulse_width8x;

volatile uint8_t recv_buf[RECV_BUF_SZ];

#define O(c) (1<<(2*c))
#define ALT_FN(c) (2<<(2*c))
#define SWD (ALT_FN(13)|ALT_FN(14))

void init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		remainders[i] = i*153;

	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD|O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7);

	GPIOB->ODR = 1<<1;
	GPIOB->MODER = O(1);
	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, 48e6/1e6);

	routing_table = table;
	for (i=0; i<ROUTING_TABLE_SIZE; i++)
		table[i] = default_route[i];
}

