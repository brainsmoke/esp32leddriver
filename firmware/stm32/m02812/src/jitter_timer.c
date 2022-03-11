#include <stdlib.h>
#include "stm32f0xx.h"

#include "util.h"
#include "jitter_timer.h"

#define O(c) (1<<(2*(c)))
#define ALT_FN(c) (2<<(2*(c)))
#define SWD (ALT_FN(13)|ALT_FN(14))
#define MASK(c) (3<<(2*(c)))

#define AFR_REG(n) ( (n)>>3 )
#define AFR_SHIFT(n) ( 1 << ( ((n)&7) * 4 ) )
#define PIN_MODE(n) (1<<(2*(n)))

#define F_CPU_MEASURED (47838000)

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

	TIM3->CCER = (0*TIM_CCER_CC1P) |
	             (1*TIM_CCER_CC1NP); /* polarity: input */

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

static void frame_set_route(int index, int mask, int timeout)
{
	frame[index] = (timeout<<3)| mask | ROUTING;
}

static void frame_set_rgb(int index, int r, int g, int b)
{
	frame_set_value(index+0, r);
	frame_set_value(index+1, g);
	frame_set_value(index+2, b);
}

void init_frame(void)
{
	int i;
	for (i=0; i<FRAME_LEN; i++)
		frame_set_value(i, 0xaa);

	frame_set_rgb(LED0, 0x0f, 0x07, 0x00);
	frame_set_rgb(LED1, 0x0f, 0x00, 0x07);
	frame_set_rgb(LED2, 0x07, 0x0f, 0x00);
	frame_set_rgb(LED3, 0x07, 0x00, 0x0f);

	frame_set_route(PACKET0, 1, 0);
	frame_set_route(PACKET1, 3, 0);
	frame_set_route(PACKET2, 7, 15);
}

void init(void)
{
	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD;

	usart1_config(48e6/115200);
	tim3_pa6_input_capture_enable();
}

void set_out_pin(int n)
{
	GPIOA->MODER &=~ ( MASK(0)|MASK(1)|MASK(2)|MASK(3) );

	if (n < 4)
		GPIOA->MODER |= O(n);
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

	init();
	init_frame();
	TIM3->CR1 |= TIM_CR1_CEN;

	for(;;)
	{
		clear_histogram();
//		uart_getchar();
		set_out_pin(2);
		bitbang(frame, 2, GPIOA, TIM3);
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
			print_32(freq);
			print("\r\n");
		}
	}

}
