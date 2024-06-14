#ifndef GPIO_DMA_H
#define GPIO_DMA_H

#include "config.h"

#ifndef DMA_WIDTH
#error "DMA_WIDTH needs to be defined (8 or 16)"
#elif DMA_WIDTH != 16 && DMA_WIDTH != 8
#error "unsupported DMA_WIDTH"
#endif

#define DMA_MASK ((1<<DMA_WIDTH)-1)

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>

#if DMA_WIDTH == 16
typedef uint16_t transfer_t;
#elif DMA_WIDTH == 8
typedef uint8_t transfer_t;
#endif

void gpio_dma_init(GPIO_TypeDef *gpio, int rate);
void gpio_dma_start(volatile transfer_t buf[], uint32_t n_transfers);

/* callbacks, implement yourself */
void gpio_half_transfer(void);
void gpio_full_transfer(void);

#endif

#endif // GPIO_DMA_H
