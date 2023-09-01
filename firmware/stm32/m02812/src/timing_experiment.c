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

void print(const char *s)
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
max_val=0xff
gamma=2.2
factor = max_val / (255.01**gamma)
gamma_map = [ int(x**gamma * factor) for x in range(256) ]
wave = [ gamma_map[int(-cos(x*pi/128)*127.5+127.5)] for x in range(256) ]
for i in range(0, 256, 16):
    print ( '\t'+' '.join('0x{:02x},'.format(x+1) for x in wave[i:i+16]) )
*/
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04,
        0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0b, 0x0d, 0x0e, 0x0f, 0x10, 0x12,
        0x13, 0x15, 0x17, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x25, 0x27, 0x29, 0x2d, 0x2f, 0x32, 0x35,
        0x38, 0x3a, 0x3d, 0x40, 0x44, 0x48, 0x4b, 0x4f, 0x52, 0x56, 0x59, 0x5d, 0x61, 0x65, 0x69, 0x6d,
        0x71, 0x76, 0x7a, 0x7d, 0x81, 0x86, 0x8b, 0x8e, 0x93, 0x96, 0x9b, 0x9e, 0xa3, 0xa7, 0xac, 0xb0,
        0xb3, 0xb7, 0xbb, 0xc0, 0xc4, 0xc8, 0xca, 0xce, 0xd2, 0xd6, 0xd8, 0xdc, 0xde, 0xe2, 0xe4, 0xe8,
        0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf5, 0xf7, 0xf7, 0xf9, 0xfb, 0xfb, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd,
        0xff, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfb, 0xfb, 0xf9, 0xf7, 0xf7, 0xf5, 0xf2, 0xf0, 0xee, 0xec,
        0xea, 0xe8, 0xe4, 0xe2, 0xde, 0xdc, 0xd8, 0xd6, 0xd2, 0xce, 0xca, 0xc8, 0xc4, 0xc0, 0xbb, 0xb7,
        0xb3, 0xb0, 0xac, 0xa7, 0xa3, 0x9e, 0x9b, 0x96, 0x93, 0x8e, 0x8b, 0x86, 0x81, 0x7d, 0x7a, 0x76,
        0x71, 0x6d, 0x69, 0x65, 0x61, 0x5d, 0x59, 0x56, 0x52, 0x4f, 0x4b, 0x48, 0x44, 0x40, 0x3d, 0x3a,
        0x38, 0x35, 0x32, 0x2f, 0x2d, 0x29, 0x27, 0x25, 0x22, 0x20, 0x1e, 0x1c, 0x1a, 0x18, 0x17, 0x15,
        0x13, 0x12, 0x10, 0x0f, 0x0e, 0x0d, 0x0b, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x06, 0x05, 0x05,
        0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
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

const char *help = (
"\r\n--------"
"\r\n  < >     increase/decrease number of leds driven" 
"\r\n  - +     adjust T0L"
"\r\n  [ ]     adjust T1L"
"\r\n  a       rotate animation"
"\r\n  h       help"
"\r\n" );

int main(void)
{
	uint32_t phase=0;

	init();

	set_out_pin(PIN);

	uint32_t t1h = T1H_DEFAULT;
	uint32_t t1l = T1L_DEFAULT;
	uint32_t n_leds = LEDS_DEFAULT;

	enum
	{
		HUE_LOW,
		HUE_MID,
		HUE_HIGH,
		MINIMAL,

		N_ANIMATIONS
	} animation = 0;

	const char *ani_name[] =
	{
		[HUE_LOW] = "hue wave [0..16]\r\n",
		[HUE_MID] = "hue wave [0..64]\r\n",
		[HUE_HIGH] = "hue wave [0..255]\r\n",
		[MINIMAL] = "minimal\r\n",
	};

	int clear_leds = 0;
	for(;;)
	{
		phase += 1;
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
				case '<': case ',':
					n_leds -= 1;
					clear_leds = 1;
					break;
				case '>': case '.':
					n_leds += 1;
					break;
				case 'a': case 'A':
					animation += 1;
					break;
				case 'h': case 'H': case '?': case '/':
					print(help);
					break;
				default:
					break;
			}
			if (t1h < T1H_MIN)
				t1h = T1H_MIN;
			if (t1h > T1H_MAX)
				t1h = T1H_MAX;
			if (t1l < T1L_MIN)
				t1l = T1L_MIN;
			if (t1l > T1L_MAX)
				t1l = T1L_MAX;
			if (n_leds < LEDS_MIN)
			{
				n_leds = LEDS_MIN;
				clear_leds = 0;
			}
			if (n_leds > LEDS_MAX)
				n_leds = LEDS_MAX;
			if (animation == N_ANIMATIONS)
				animation = 0;
			print("--------\r\nT1H: ");
			print_32(t1h);
			print("\r\nT1L: ");
			print_32(t1l);
			print("\r\n\r\nleds: ");
			print_32(n_leds);
			print("\r\nanimation: ");
			print(ani_name[animation]);
			print("\r\n");
		}

		int i;
		for (i=0; i<n_leds; i++)
		{
			int r,g,b;
			switch (animation)
			{
				case HUE_LOW:
					r = (fade[ ( ((i*127+phase)>>2)    ) & 0xff ]>>4)|1; /* always set low bit */
					g = (fade[ ( ((i*127+phase)>>2)+85 ) & 0xff ]>>4)|1; /* to increase chance */
					b = (fade[ ( ((i*127+phase)>>2)+170) & 0xff ]>>4)|1; /* of visible glitches */
					break;
				case HUE_MID:
					r = (fade[ ( ((i*127+phase)>>2)    ) & 0xff ]>>2)|1;
					g = (fade[ ( ((i*127+phase)>>2)+85 ) & 0xff ]>>2)|1;
					b = (fade[ ( ((i*127+phase)>>2)+170) & 0xff ]>>2)|1;
					break;
				case HUE_HIGH:
					r = (fade[ ( ((i*127+phase)>>2)    ) & 0xff ])|1;
					g = (fade[ ( ((i*127+phase)>>2)+85 ) & 0xff ])|1;
					b = (fade[ ( ((i*127+phase)>>2)+170) & 0xff ])|1;
					break;
				case MINIMAL:
					r=g=b=1;
					break;
				default:
					r=g=b=0;
					break;
			}
			set_rgb(i, r, g, b);
		}
		for (i=0; i<clear_leds; i++)
				set_rgb(n_leds+i, 0, 0, 0);
		bitbang(framebuffer, (n_leds+clear_leds)*3, t1h, t1l);
		clear_leds = 0;
	}
}
