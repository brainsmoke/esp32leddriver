#ifndef JITTER_TIMER_H
#define JITTER_TIMER_H

#include "util.h"

#define ROUTING (0)
#define VALUE   (0x100)

#define HISTOGRAM_MAX (0x80)

#define LED0  (0)
#define LED1  (4)
#define LED2  (8)
#define LED3 (12)

#define PACKET1  (3)
#define PACKET2  (7)
#define PACKET3 (11)

#define BITBANG_NATIVE_CYCLES (70)

#define MEASURE_LEN            (1024)
#define MEASURE_PERIODS        (MEASURE_LEN*8-1)
#define MEASURE_NATIVE_CYCLES  (BITBANG_NATIVE_CYCLES*MEASURE_PERIODS)
#define FRAME_LEN              (MEASURE_LEN+PACKET3+1)

#ifndef __ASSEMBLER__

#include <stdint.h>

extern uint32_t histogram[HISTOGRAM_MAX+1];
void bitbang(uint16_t *buf, uint32_t pin, volatile GPIO_TypeDef *gpio, volatile TIM_TypeDef *timer);

#endif

#endif // JITTER_TIMER_H
