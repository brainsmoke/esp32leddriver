#ifndef BITBANG_H
#define BITBANG_H

/* platform constants */

#define GPIO_ODR_OFFSET (0x14)
#define GPIO_BSRR_OFFSET (0x18)

/* GPIO, BSRR */

#define CLEAR(X) (X<<16)
#define SET(X)   (X)

/* GPIO A */
#define PIN_DATA_0             (0)
#define PIN_DATA_1             (1)
#define PIN_DATA_2             (2)
#define PIN_DATA_3             (3)
#define PIN_CLK                (4)
#define PIN_LATCH              (5)
#define PIN_NOT_OUTPUT_ENABLE  (6)

#define BIT_DATA_0             (1<<PIN_DATA_0)
#define BIT_DATA_1             (1<<PIN_DATA_1)
#define BIT_DATA_2             (1<<PIN_DATA_2)
#define BIT_DATA_3             (1<<PIN_DATA_3)
#define BIT_CLK                (1<<PIN_CLK)
#define BIT_LATCH              (1<<PIN_LATCH)
#define BIT_NOT_OUTPUT_ENABLE  (1<<PIN_NOT_OUTPUT_ENABLE)

#define MASK_CLK (BIT_CLK)
#define MASK_DATA (BIT_DATA_0|BIT_DATA_1|BIT_DATA_2|BIT_DATA_3)

/* static constants (code changes needed to change this) */

#define N_CHANNELS           (4)
#define N_CHIPS_PER_CHANNEL  (4)
#define N_PINS_PER_CHIP      (16)

/* derived constants */

#define N_BITS_PER_CHANNEL   (N_CHIPS_PER_CHANNEL*N_PINS_PER_CHIP)
#define N_LEDS               (N_BITS_PER_CHANNEL*N_CHANNELS)

#ifndef __ASSEMBLER__

#include <stdint.h>
#include "stm32f0xx.h"

#define GPIO_OUT (GPIOA)
#define GPIO_OUT_BSRR (GPIOA->BSRR)

/* sends 64x (max 8) bits parallel at ~1/6th the clockspeed to GPIO[0-7]
 * gpio is the GPIO base address
 * 
 * probably 340 cycles excluding call
 */
void bitbang64_clk_stm32(uint8_t *buffer, volatile uint16_t *gpio);
void bitbang64_clk_no_enable_stm32(uint8_t *buffer, volatile uint16_t *gpio);

void write_wait_write(volatile uint32_t *addr, uint32_t pre_data, uint32_t post_data, uint32_t cycles);


int get_u8(void);
int get_u16le(void);
void init_io(void);

static inline void io_poll(void) {}

#endif

#endif // BITBANG_H
