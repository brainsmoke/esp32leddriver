#include <stdlib.h>
#include "stm32f0xx.h"

//#include "util.h"
#include "jitter_timer.h"

#define O(c) (1<<(2*(c)))
#define ALT_FN(c) (2<<(2*(c)))
#define SWD (ALT_FN(13)|ALT_FN(14))
#define MASK(c) (3<<(2*(c)))

#define AFR_REG(n) ( (n)>>3 )
#define AFR_SHIFT(n) ( 1 << ( ((n)&7) * 4 ) )
#define PIN_MODE(n) (1<<(2*(n)))

#define INPUT_CAPTURE_PIN (6)

#define F_CPU_MEASURED (47838000)

void clock48mhz_xtal16mhz(void)
{
	/* 1 wait state required for flash accesses at clockspeeds > 24MHz */
	/* enable prefetching */
	FLASH->ACR |= (1 * FLASH_ACR_LATENCY)  |  FLASH_ACR_PRFTBE;

	/* Turn on external xtal */
	RCC->CFGR2 = RCC_CFGR2_PREDIV1_DIV2;
	RCC->CR |= RCC_CR_CSSON | RCC_CR_HSEON;

	/* reading the RCC_CR_HSERDY flag is safe only after 6 cycles */
	__ASM volatile ("nop; nop; nop; nop; nop;   nop; nop; nop; nop; nop;");

	while( !(RCC->CR & RCC_CR_HSERDY) );

	/* HSE is 16MHz x2 DIV, PLL has a 2x divider & config a 12x multplier -> 48MHz */
	RCC->CFGR = ( RCC->CFGR & ~RCC_CFGR_PLLMULL ) | RCC_CFGR_PLLMULL12;

	/* turn on PLL */
	RCC->CR |= RCC_CR_PLLON;
	while( !(RCC->CR & RCC_CR_PLLRDY) );

	/* switch to PLL for system clock */
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}


void tim3_pa6_input_capture_enable(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    GPIOA->MODER |= ALT_FN(6); /* alternate function mode for PA6 */
	GPIOA->AFR[AFR_REG(6)] |= 1*AFR_SHIFT(6); /* pa6 is tim3_ch1 */

	TIM3->CR1 = (0*TIM_CR1_CKD_0) | /* no clock division */
	            (0*TIM_CR1_CMS_0) |
	            (0*TIM_CR1_DIR)   |
	            (0*TIM_CR1_OPM)   |
	            (0*TIM_CR1_URS)   |
	            (0*TIM_CR1_UDIS)  |
	            (0*TIM_CR1_CEN)   ; /* don't start counting yet */

	TIM3->CCMR1 = (1*TIM_CCMR1_CC1S_0)   | /* select input compare 1 register */
	              (2*TIM_CCMR1_IC1F_0)   | /* 4 cycles input filter           */
	              (0*TIM_CCMR1_IC1PSC_0) ; /* capture every pulse             */

	TIM3->CCER = (0*TIM_CCER_CC1P) | /* polarity: raising */
	             (0*TIM_CCER_CC1NP);

	TIM3->CCER |= (1*TIM_CCER_CC1E); /* input capture enable */

}

void usart1_config(long baudrate_prescale)
{
    GPIOA->MODER |= ALT_FN(9)|ALT_FN(10); /* alternate function mode for PA9/pa10 */
    GPIOA->AFR[AFR_REG(9)]  |= 1*AFR_SHIFT(9);  /* mux PA9 to usart1_tx */
    GPIOA->AFR[AFR_REG(10)] |= 1*AFR_SHIFT(10); /* mux PA10 to usart1_rx */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    USART1->CR1 = 0;
    USART1->BRR = baudrate_prescale;
    USART1->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
}

void uart_putchar(int c)
{
	while ( !(USART1->ISR & USART_ISR_TXE) );
	USART1->TDR = c & 0xff;
}

int uart_getchar(void)
{
	while ( !(USART1->ISR & USART_ISR_RXNE) );
	return USART1->RDR & 0xff;
}

void print(char *s)
{
	while (*s)
		uart_putchar(*s++);
}

const char *hex = "0123456789abcdef";

static void print_hex32(uint32_t x)
{
	uart_putchar( hex[(x>>28)& 0xf] );
	uart_putchar( hex[(x>>24)& 0xf] );
	uart_putchar( hex[(x>>20)& 0xf] );
	uart_putchar( hex[(x>>16)& 0xf] );
	uart_putchar( hex[(x>>12)& 0xf] );
	uart_putchar( hex[(x>>8) & 0xf] );
	uart_putchar( hex[(x>>4) & 0xf] );
	uart_putchar( hex[(x   ) & 0xf] );
}

static void print_32(uint32_t x)
{
	uint32_t i=1000000000;

	while (x < i)
	{
//		uart_putchar(' ');
		i/=10;
	}
	if (i==0)
		i = 1;

	while (i)
	{
		uint32_t d = x/i;
		uart_putchar( hex[d] );
		x-=d*i;
		i/=10;
	}
}

static void print_hex8(uint8_t x)
{
	uart_putchar( hex[(x>>4) & 0xf] );
	uart_putchar( hex[(x   ) & 0xf] );
}

uint16_t frame[FRAME_LEN];

static void frame_set_value(int index, int v)
{
	frame[index] = (v&0xff) | VALUE;
}

static void frame_set_route(int index, int route, int timeout)
{
	frame[index] = (timeout<<3)| ((1<<(route-1))&7) | ROUTING;
}

static void frame_set_rgb(int index, int r, int g, int b)
{
	frame_set_value(index+0, r);
	frame_set_value(index+1, g);
	frame_set_value(index+2, b);
}

const uint8_t fade[] =
{
/*
from math import pi, cos
max_val=0x30
gamma=2.2
factor = max_val / (255.**gamma)
gamma_map = [ int(x**gamma * factor) for x in range(256) ]
wave = [ gamma_map[int(-cos(x*pi/64)*127.5+127.5)] for x in range(128) ]
for i in range(0, 256, 16):
    print ( '\t'+' '.join('0x{:02x},'.format(x+1) for x in wave[i:i+16]) )
*/
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
	0x0b, 0x0c, 0x0d, 0x0f, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17, 0x19, 0x1b, 0x1c, 0x1e, 0x1f, 0x21,
	0x22, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x2f, 0x30, 0x30, 0x30,
	0x31, 0x30, 0x30, 0x30, 0x2f, 0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x26, 0x25, 0x24,
	0x22, 0x21, 0x1f, 0x1e, 0x1c, 0x1b, 0x19, 0x17, 0x16, 0x14, 0x13, 0x11, 0x10, 0x0f, 0x0d, 0x0c,
	0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x05, 0x04, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};

void init_frame(void)
{
	int i;
	for (i=0; i<FRAME_LEN; i++)
		frame_set_value(i, 0x55);

}

void set_frame_info(int pin, int phase)
{
	switch (pin)
	{
		case 4:
			frame_set_rgb(LED0, fade[phase], fade[phase], fade[phase]);

			frame_set_route(PACKET1, 1, 0);
			frame_set_rgb(LED1, fade[phase], 0, 0);

			frame_set_route(PACKET3, 2, 15);         /* sense pin last */
			frame_set_rgb(LED3, 0, fade[phase], 0);

			frame_set_route(PACKET2, 3, 0);
			frame_set_rgb(LED2, 0, 0, fade[phase]);
			break;
		case 5:
			frame_set_rgb(LED0, fade[phase], fade[phase], fade[phase]);

			frame_set_route(PACKET3, 1, 15);         /* sense pin last */
			frame_set_rgb(LED3, fade[phase], 0, 0);

			frame_set_route(PACKET1, 2, 0);
			frame_set_rgb(LED1, 0, fade[phase], 0);

			frame_set_route(PACKET2, 3, 0);
			frame_set_rgb(LED2, 0, 0, fade[phase]);
			break;
		case 6:
			frame_set_rgb(LED0, fade[phase], fade[phase], fade[phase]); /* no sense pin */

			frame_set_route(PACKET1, 1, 0);
			frame_set_rgb(LED1, fade[phase], 0, 0);

			frame_set_route(PACKET2, 2, 0);
			frame_set_rgb(LED2, 0, fade[phase], 0);

			frame_set_route(PACKET3, 3, 15);
			frame_set_rgb(LED3, 0, 0, fade[phase]);
			break;
		case 7:
			frame_set_rgb(LED0, fade[phase], fade[phase], fade[phase]);

			frame_set_route(PACKET1, 1, 0);
			frame_set_rgb(LED1, fade[phase], 0, 0);

			frame_set_route(PACKET2, 2, 0);
			frame_set_rgb(LED2, 0, fade[phase], 0);

			frame_set_route(PACKET3, 3, 15);         /* sense pin last */
			frame_set_rgb(LED3, 0, 0, fade[phase]);
			break;
	}
}

void init(void)
{
	clock48mhz_xtal16mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD;

	usart1_config(48e6/115200);
	tim3_pa6_input_capture_enable();
}

void set_out_pin(int n)
{
	uint32_t mode = GPIOA->MODER & ~( MASK(4)|MASK(5)|MASK(6)|MASK(7) );

	if (n != INPUT_CAPTURE_PIN)
		mode |= ALT_FN(INPUT_CAPTURE_PIN); /* alternate function mode for PA6 */

	if ( (n >= 4) && (n < 8) )
		mode |= O(n);

	GPIOA->MODER = mode;
}

void set_out_all_pins(void)
{
	uint32_t mode = GPIOA->MODER & ~( MASK(4)|MASK(5)|MASK(6)|MASK(7) );
	GPIOA->MODER = mode | O(4)|O(5)|O(6)|O(7) ;
}

void clear_histogram(void)
{
	int i;
	for (i=0; i<HISTOGRAM_MAX+1; i++)
		histogram[i]=0;
}

void print_histogram(void)
{
	print("----\r\n");
	int i;
	for (i=1; i<HISTOGRAM_MAX+1; i++)
		if (histogram[i])
		{
			print_hex8(i);
			print(": ");
			print_hex32(histogram[i]);
			print("\r\n");
		}
}


int main(void)
{
	int pin, phase;

	init();
	init_frame();
	TIM3->CR1 |= TIM_CR1_CEN;

	for(;;)
	{
		for (pin=4; pin<8; pin++)
		{
			set_out_all_pins();
			bitbang_clear(GPIOA, 0x00f0);
			set_out_pin(pin);
			uint32_t max_freq = 0, min_freq = UINT32_MAX;
			for (phase=0; phase<128; phase++)
			{
				set_frame_info(pin, phase);
				clear_histogram();
				bitbang(frame, pin, GPIOA, TIM3);

				if (pin != INPUT_CAPTURE_PIN)
				{
					uint32_t cycles11 = histogram[65]+histogram[66]+histogram[67];
					cycles11 += histogram[64]+histogram[68]; /* probably not needed */
					uint32_t cycles12 = histogram[71]+histogram[72]+histogram[73];
					cycles12 += histogram[70]+histogram[74]; /* probably not needed */
					if (cycles11 + cycles12 == MEASURE_PERIODS)
					{
						uint32_t freq = F_CPU_MEASURED/2000;
						freq *= 11*cycles11+12*cycles12;
						freq /= MEASURE_NATIVE_CYCLES;
						freq *= 2000;
						if (min_freq > freq)
							min_freq = freq;
						if (max_freq < freq)
							max_freq = freq;
					}
				}
			}
			print("pin = ");
			print_32(pin);
			if (max_freq == 0)
			{
				print(" -");
			}
			else
			{
				print(", min = ");
				print_32(min_freq);
				print(", max = ");
				print_32(max_freq);
			}
			print(";\r\n");
		}
	}
}
