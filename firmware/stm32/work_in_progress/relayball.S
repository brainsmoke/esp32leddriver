
#include <relayball.h>


#define rCARRY   r0
#define rINBUF   r1
#define rOUTBUF  r2
#define rMASK    r7
#define rDH      r3 /* data out, bits 7..4, dithering remainder 7..4 */
#define rDL      r4 /* data out, bits 3..0, dithering remainder 3..0 */
#define rSH      r5
#define rSL      r6


/*  0 +  5 */ push {r4-r7}

.macro dither_single_byte

/*  0 +  2 */  ldr  rDL, [rOUTBUF, #4]
/*  2 +  3 */  ldm rINBUF! {rSH, rSL}

                                       /* _____________________bits______________________ */
                                       /*   11    3     10    2      9    1      8    0   */
/*  5 +  1 */  bics rDL, rMASK         /* [0000 RRRR] [0000 RRRR] [0000 RRRR] [0000 RRRR] */
/*  6 +  1 */  orrs rDL, rCARRY        /* [0000 RRRR] [0000 RRRR] [0000 RRRR] [CCCC RRRR] */
/*  7 +  1 */  eors rDL, rSL           /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/*  8 +  1 */  bics rSL, rDL           /* [0000 CCCC] [0000 CCCC] [0000 CCCC] [CCCC CCCC] */
/*  9 +  1 */  lsls rCARRY, rSL, #8    /* [0000 CCCC] [0000 CCCC] [CCCC CCCC]             */

/* 10 +  1 */  eors rDL, rCARRY        /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 11 +  1 */  bics rCARRY, rDL        /* [0000 CCCC] [0000 CCCC] [CCCC CCCC]             */
/* 12 +  1 */  orrs rSL, rCARRY        /* [0000 CCCC] [???? ????] [???? ????] [???? ????] */
/* 13 +  1 */  lsls rCARRY, #8         /* [0000 CCCC] [CCCC CCCC]                         */

/* 14 +  1 */  eors rDL, rCARRY        /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 15 +  1 */  bics rCARRY, rDL        /* [0000 CCCC] [CCCC CCCC]                         */
/* 16 +  1 */  orrs rSL, rCARRY        /* [CCCC CCCC] [???? ????] [???? ????] [???? ????] */
/* 17 +  1 */  lsls rCARRY, #8         /* [CCCC CCCC]                                     */

/* 18 +  1 */  eors rDL, rCARRY        /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 19 +  1 */  bics rCARRY, rDL        /* [CCCC CCCC]                                     */
/* 20 +  1 */  orrs rSL, rCARRY        /* [CCCC CCCC] [???? ????] [???? ????] [???? ????] */
/* 21 +  1 */  lsrs rSL, #24           /*                                     [CCCC CCCC] */

/* 22 +  2 */  ldr  rDL, [rOUTBUF, #0]
                                       /* _____________________bits______________________ */
                                       /*   15    7     14    6     13    5     12    4   */
/* 24 +  1 */  bics rDH, rMASK         /* [0000 RRRR] [0000 RRRR] [0000 RRRR] [0000 RRRR] */
/* 25 +  1 */  eors rDH, rSH           /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 26 +  1 */  bics rSH, rDH           /* [0000 CCCC] [0000 CCCC] [0000 CCCC] [CCCC CCCC] */
/* 27 +  1 */  lsls rCARRY, rSH, #8    /* [0000 CCCC] [0000 CCCC] [CCCC CCCC]             */
/* 28 +  1 */  orrs rCARRY, rSL        /* [0000 CCCC] [0000 CCCC] [CCCC CCCC] [CCCC CCCC] */


/* 29 +  1 */  eors rDH, rCARRY        /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 30 +  1 */  bics rCARRY, rDH        /* [0000 CCCC] [0000 CCCC] [CCCC CCCC] [CCCC CCCC] */
/* 31 +  1 */  orrs rSH, rCARRY        /* [CCCC CCCC] [???? ????] [???? ????] [???? ????] */
/* 32 +  1 */  lsls rCARRY, #8         /* [0000 CCCC] [CCCC CCCC] [CCCC CCCC]             */

/* 33 +  1 */  eors rDH, rCARRY        /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 34 +  1 */  bics rCARRY, rDH        /* [0000 CCCC] [CCCC CCCC] [CCCC CCCC]             */
/* 35 +  1 */  orrs rSH, rCARRY        /* [CCCC CCCC] [???? ????] [???? ????] [???? ????] */
/* 36 +  1 */  lsls rCARRY, #8         /* [CCCC CCCC] [CCCC CCCC]                         */

/* 37 +  1 */  eors rDH, rCARRY        /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 38 +  1 */  bics rCARRY, rDH        /* [CCCC CCCC] [CCCC CCCC]                         */
/* 39 +  1 */  orrs rSH, rCARRY        /* [CCCC CCCC] [???? ????] [???? ????] [???? ????] */
/* 40 +  1 */  lsls rCARRY, #8         /* [CCCC CCCC]                                     */

/* 41 +  1 */  eors rDH, rCARRY        /* [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] [DDDD RRRR] */
/* 42 +  1 */  bics rCARRY, rDH        /* [CCCC CCCC]                                     */
/* 43 +  1 */  orrs rCARRY, rSH        /* [CCCC CCCC] [???? ????] [???? ????] [???? ????] */
/* 44 +  1 */  lsrs rCARRY, #24        /* [CCCC CCCC]                                     */

/* 45 +  3 */  stm  rOUTBUF!, {rDH, rDL}

.endm

/*   0 + 48 */  dither_single_byte
/*  48 + 48 */  dither_single_byte
/*  96 + 48 */  dither_single_byte

/* 144 + 48 */  dither_single_byte
/* 192 + 48 */  dither_single_byte
/* 240 + 48 */  dither_single_byte

/* 288 + 48 */  dither_single_byte
/* 336 + 48 */  dither_single_byte
/* 384 + 48 */  dither_single_byte

/* 432 + 48 */  dither_single_byte
/* 480 + 48 */  dither_single_byte
/* 528 + 48 */  dither_single_byte

/* 576 +  2 */  adds rOUTBUF, #8


/*  xx +  5 */  pop {r4-r7}
/*  xx +  5 */  bx lr

	.size	bitbang16, .-bitbang16

