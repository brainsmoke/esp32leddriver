
#include <stdlib.h>
#include "stm32f0xx.h"

#include "dither8.h"
#include "util.h"

uint8_t framebuf[N_BYTES];
//static uint8_t *outbuf;
uint8_t *frame;

void SysTick_Handler(void)
{
	bitbang(framebuf, &GPIOA->ODR);
}

void init(void)
{
	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD|O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7);
	enable_sys_tick(F_SYS_TICK_CLK/400);
}

int main(void)
{
	init();

	for(;;);

	return 0;
}

