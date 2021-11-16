#ifndef UARTBALL16_H
#define UARTBALL16_H

#ifndef N_VALUES_PER_LED
#define N_VALUES_PER_LED  (6)
#endif

#ifndef N_LEDS_PER_STRIP
#define N_LEDS_PER_STRIP (8)
#endif

/* static constants */

#define N_STRIPS (8) /* code changes needed to change this */

#define N_BAUDS_PER_VALUE (10)

/* derived constants */

#define N_VALUES_PER_STRIP (N_LEDS_PER_STRIP*N_VALUES_PER_LED)
#define N_LEDS (N_LEDS_PER_STRIP*N_STRIPS)
#define N_VALUES (N_VALUES_PER_STRIP*N_STRIPS)
#define N_BAUDS (N_VALUES_PER_STRIP*N_BAUDS_PER_VALUE)

#define BAUDRATE (38400)

#endif // UARTBALL16_H
