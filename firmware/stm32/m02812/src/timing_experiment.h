#ifndef TIMING_EXPERIMENT_H
#define TIMING_EXPERIMENT_H

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (3)
#endif

#ifndef N_LEDS
#define N_LEDS (1024)
#endif

#define N_VALUES (N_VALUES_PER_LED*N_LEDS)
#define N_BYTES (N_VALUES)

#define PIN (4)
#define GPIOA_ODR (0x48000014)

#define T1H_MIN ( (6+15) )
#define T1L_MIN ( (9+2) )

#define T1H_MAX ( 60 )
#define T1L_MAX ( 60 )

#define T1H_DEFAULT (42)
#define T1L_DEFAULT (18)

#define LEDS_DEFAULT (128)
#define LEDS_MIN (1) /* bitbang routine needs to output at least one byte */
#define LEDS_MAX (N_LEDS)

#ifndef __ASSEMBLER__

#include <stdint.h>

/* t1h/t1l in seconds/384e6 */
void bitbang(uint8_t *buf, uint32_t size, uint32_t t1h, uint32_t t1l);

#endif

#endif // TIMING_EXPERIMENT_H
