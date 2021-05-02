#ifndef FSM_H
#define FSM_H

#define GOOD          (0*7)
#define GOOD_00       (1*7)
#define GOOD_01_FE    (2*7)
#define GOOD_FF       (3*7)
#define GOOD_FFFF     (4*7)
#define GOOD_FFFFFF   (5*7)
#define BAD           (6*7)
#define BAD_FF        (7*7)
#define BAD_FFFF      (8*7)
#define BAD_FFFFFF    (9*7)

#define STATE_COUNT   (10*7)

#define GOOD_RETURN  0xf0
#define ROUTE        0xf1
#define TIMING       0xf2

#define BAD_RETURN   0xff

#define IN_00     0
#define IN_01_EF  1
#define IN_F0     2
#define IN_F1     3
#define IN_F2     4
#define IN_F3_FE  5
#define IN_FF     6

#ifndef __ASSEMBLER__

#include <stdint.h>

extern const uint8_t fsm[STATE_COUNT];

#endif

#endif //FSM_H
