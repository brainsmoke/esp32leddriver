
#include "stm32f0xx.h"

#include "obegraensad.h"
#include "util.h"

#define RECV_BUF_SZ (128)
static volatile uint8_t recv_buf[RECV_BUF_SZ];

int get_u8(void)
{
	static uint32_t last = 0;
	if (last == 0)
		last = RECV_BUF_SZ;
	while (last == DMA1_Channel3->CNDTR);
	last--;
	return recv_buf[RECV_BUF_SZ-1-last];
}

int get_u16le(void)
{
	int c;
	c  =   get_u8();
	c |= ( get_u8() << 8 );
	return c;
}

void init_io(void)
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
}

