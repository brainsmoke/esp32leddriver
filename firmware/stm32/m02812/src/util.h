#ifndef UTIL_H
#define UTIL_H

#define F_CPU (48e6)
#define F_SYS_TICK_CLK (F_CPU/8)

void clock48mhz(void);
void enable_sys_tick(uint32_t ticks);
void usart1_rx_pa10_dma3_enable(volatile uint8_t *buf, uint32_t size, long baudrate_prescale);

#endif // UTIL_H
