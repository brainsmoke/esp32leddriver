#include <stdlib.h>
#include "stm32f0xx.h"

#include "timing_experiment.h"

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
	if (USART1->ISR & USART_ISR_RXNE)
		return USART1->RDR & 0xff;
	else
		return -1;
}

void print(char *s)
{
	while (*s)
		uart_putchar(*s++);
}

const char *hex = "0123456789abcdef";

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

uint8_t framebuffer[N_VALUES_PER_LED*N_LEDS];

static void set_rgb(unsigned int index, int r, int g, int b)
{
	if (index < N_LEDS)
	{
		framebuffer[index*3+0] = r;
		framebuffer[index*3+1] = g;
		framebuffer[index*3+2] = b;
	}
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

void init(void)
{
	clock48mhz_xtal16mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD;

	usart1_config(48e6/115200);
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

int main(void)
{
	int phase;

	init();

	set_out_pin(PIN);

	uint32_t t1h = T1H_DEFAULT;
	uint32_t t1l = T1L_DEFAULT;

	for(;;)
	{
		for (phase=0; phase<128; phase++)
		{
			int c = uart_getchar();
			if (c != -1)
			{
				switch (c)
				{
					case '-': case '_':
						t1h -= 1;
						break;
					case '+': case '=':
						t1h += 1;
						break;
					case '[': case '{':
						t1l -= 1;
						break;
					case ']': case '}':
						t1l += 1;
						break;
				}
				if (t1h < T1H_MIN)
					t1h = T1H_MIN;
				if (t1l < T1L_MIN)
					t1l = T1L_MIN;
				if (t1h > T1H_MAX)
					t1h = T1H_MAX;
				if (t1l > T1L_MAX)
					t1l = T1L_MAX;
				print("--------\r\nT1H: ");
				print_32(t1h);
				print("\r\nT1L: ");
				print_32(t1l);
				print("\r\n");
			}

			int i;
			for (i=0; i<N_LEDS; i++)
			{
//				set_rgb(i, 0xc3, 0xc3, 0xc3);
				set_rgb(i, (fade[ (i+phase    ) & 0x7f ] >> 2)|1, /* always set low bit  */
				           (fade[ (i+phase+85 ) & 0x7f ] >> 2)|1, /* to increase chance  */
				           (fade[ (i+phase+170) & 0x7f ] >> 2)|1);/* of visible glitches */
			}
			bitbang(framebuffer, N_VALUES, t1h, t1l);
		}
	}
}
