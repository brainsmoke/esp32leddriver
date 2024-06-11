#ifndef UTIL_H
#define UTIL_H

#define F_CPU (48000000)
#define F_SYS_TICK_CLK (F_CPU/8)

#define O(c) (1<<(2*(c)))
#define ALT_FN(c) (2<<(2*(c)))
#define SWD (ALT_FN(13)|ALT_FN(14))
#define MASK(c) (3<<(2*(c)))

#define AFR_REG(n) ( (n)>>3 )
#define AFR_SHIFT(n) ( 1 << ( ((n)&7) * 4 ) )

#ifndef __ASSEMBLER__

#include <stdint.h>

void clock48mhz(void);
void enable_sys_tick(uint32_t ticks);
void usart1_rx_pa10_dma3_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale);
void usart1_rx_dma3_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale);

void usart2_rx_pa3_dma5_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale);
void usart2_rx_dma5_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale);

#endif

#endif // UTIL_H
