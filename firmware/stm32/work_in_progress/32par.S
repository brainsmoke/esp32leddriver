
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

.macro low_byte_dither_half_adder

/*  0 +  2 */  ldm  rINBUF!, {rCARRY}
/*  2 +  2 */  ldr  rSCRATCH0, [rREMBUF]
/*  4 +  1 */  eors rSCRATCH0, rCARRY
/*  5 +  1 */  bics rCARRY, rSCRATCH0
/*  6 +  2 */  stm  rREMBUF!, {rSCRATCH0}
/*  8      */

.endm

.macro low_byte_dither_full_adder

/*  0 +  2 */  ldm  rINBUF!, {rSCRATCH1}
/*  2 +  2 */  ldr  rSCRATCH0, [rREMBUF]
/*  4 +  1 */  eors rSCRATCH0, rSCRATCH1
/*  5 +  1 */  bics rSCRATCH1, rSCRATCH0
/*  6 +  1 */  eors rSCRATCH0, rCARRY
/*  7 +  1 */  bics rCARRY, rSCRATCH0
/*  8 +  1 */  orrs rCARRY, rSCRATCH1
/*  9 +  2 */  stm  rREMBUF!, {rSCRATCH0}
/* 11      */

.endm

.macro low_byte_dither_calc_carry

/*  0 +  8 */  low_byte_dither_half_adder
/*  8 + 11 */  low_byte_dither_full_adder
/* 19 + 11 */  low_byte_dither_full_adder
/* 30 + 11 */  low_byte_dither_full_adder
/* 41 + 11 */  low_byte_dither_full_adder
/* 52 + 11 */  low_byte_dither_full_adder
/* 63 + 11 */  low_byte_dither_full_adder
/* 74 + 11 */  low_byte_dither_full_adder
/* 85      */

.endm

.macro high_byte_dither

/*  0 +  3 */  ldm  rINBUF!, {rSCRATCH0, rSCRATCH1, rSCRATCH2, rSCRATCH3}
/*  3 +  1 */  eors rSCRATCH0, rCARRY
/*  4 +  1 */  bics rCARRY, rSCRATCH0
/*  5 +  1 */  eors rSCRATCH1, rCARRY
/*  6 +  1 */  bics rCARRY, rSCRATCH1
/*  7 +  1 */  eors rSCRATCH2, rCARRY
/*  8 +  1 */  bics rCARRY, rSCRATCH2
/*  9 +  1 */  eors rSCRATCH3, rCARRY
/* 10 +  1 */  bics rCARRY, rSCRATCH3
/* 11 +  5 */  push {rSCRATCH0, rSCRATCH1, rSCRATCH2, rSCRATCH3}


/* XX +  2 */  ldm  rINBUF!, {rBITx}
/* XX +  2 */  ldr  rREM, [rREMBUF]
/* XX +  1 */  eors rREM, rBITx
/* XX +  1 */  bics rBITx, rREM
/* XX +  1 */  eors rREM, rCARRY
/* XX +  1 */  bics rCARRY, rREM
/* XX +  1 */  orrs rrCARRY, rBITx
/* XX +  2 */  stm  rREMBUF!, {rREM}




/* XX +  2 */  ldr  rREMx, [rREMBUF, #4]
/* XX +  1 */  eors rREMx, rB1
/* XX +  1 */  bics rB1, rREMx
/* XX +  2 */  str  rREMx, [rREMBUF, #4]

/* XX +  2 */  ldr  rREMx, [rREMBUF, #8]
/* XX +  1 */  eors rREMx, rB2
/* XX +  1 */  bics rB2, rREMx
/* XX +  2 */  str  rREMx, [rREMBUF, #8]

/* XX +  2 */  ldr  rREMx, [rREMBUF, #12]
/* XX +  1 */  eors rREMx, rB3
/* XX +  1 */  bics rB3, rREMx
/* XX +  2 */  str  rREMx, [rREMBUF, #12]

/* XX +  5 */  ldm  rINBUF!, {rB4, rB5, rB6, rB7}

/* XX +  2 */  ldr  rREMx, [rREMBUF, #16]
/* XX +  1 */  eors rREMx, rB4
/* XX +  1 */  bics rB4, rREMx
/* XX +  2 */  str  rREMx, [rREMBUF, #16]

/* XX +  2 */  ldr  rREMx, [rREMBUF, #20]
/* XX +  1 */  eors rREMx, rB5
/* XX +  1 */  bics rB0, rREMx
/* XX +  2 */  str  rREMx, [rREMBUF, #20]

/* XX +  2 */  ldr  rREMx, [rREMBUF, #24]
/* XX +  1 */  eors rREMx, rB6
/* XX +  1 */  bics rB6, rREMx
/* XX +  2 */  str  rREMx, [rREMBUF, #24]

/* XX +  2 */  ldr  rREMx, [rREMBUF, #28]
/* XX +  1 */  eors rREMx, rB7
/* XX +  1 */  bics rB7, rREMx
/* XX +  2 */  str  rREMx, [rREMBUF, #28]



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


.macro dither_half_adder_2cycles reg_accum reg_carry

/*
 * 
 * A --------\\---..  .------------- A'
 *            ))xor )-|
 *        .--//---''  '--O|---.
 *        |               |bic )-. 
 * carry -+---------------|---'  '-- carry'
 *
 */

/*  0 +  1 */  eors \reg_acc, \reg_carry
/*  1 +  1 */  bics \reg_carry, \reg_acc
/*  2      */

.endm

.macro dither_full_adder_5cycles reg_accum reg_src reg_carry

/*
 * B -.--------------|---. 
 *    |              |bic )---------------.
 *    '-\\---..  .--O|---'                |
 *       ))xor )-|                        |
 * A ---//---''  '--\\---..  .----------- | ------------ A'
 *                   ))xor )-|            |
 *               .--//---''  '--O|---.    '-\--..
 *               |               |bic )-.    )or )-.
 * carry --------+---------------|---'  '---/--''  '--- carry'
 *
 */

/*  0 +  1 */  eors \reg_acc, \reg_src
/*  1 +  1 */  bics \reg_src, \reg_acc
/*  2 +  1 */  eors \reg_acc, \reg_carry
/*  3 +  1 */  bics \reg_carry, \reg_acc
/*  4 +  1 */  orrs \reg_carry, \reg_src
/*  5      */

.endm

.macro dither_bit_16cycles bit

/*  0 +  1 */  mov rSCRATCH, hiREMBUF
/*  1 +  2 */  ldrb rOUT, [rSCRATCH, #\bit]
/*  3 +  1 */  mov rSCRATCH, hiINBUF
/*  4 +  2 */  ldrb rSCRATCH, [rSCRATCH, #\bit]
/*  6 +  1 */  ands rOUT, #0x0f
/*  7 +  5 */  dither_full_adder_5cycles rOUT, rSCRATCH, rCARRY
/* 12 +  1 */  mov rSCRATCH, hiREMBUF
/* 13 +  2 */  strb rOUT, [rSCRATCH, #\bit]
/* 15 +  1 */  nop
/* 16      */

.endm

.macro dither_bit0_16cycles

/*  0 +  1 */  movs rSCRATCH, #8
/*  1 +  1 */  add hiINBUF, rSCRATCH
/*  2 +  1 */  add hiREMBUF, rSCRATCH
/*  3 +  1 */  mov rSCRATCH, hiREMBUF
/*  4 +  2 */  ldrb rOUT, [rSCRATCH, #0]
/*  6 +  1 */  ands rOUT, #0x0f
/*  7 +  1 */  shls rCARRY, #4
/*  8 +  1 */  orrs rCARRY, rOUT
/*  9 +  1 */  mov rSCRATCH, hiINBUF
/* 10 +  2 */  ldrb rOUT, [rSCRATCH, #0]
/* 12 +  2 */  dither_full_adder_2cycles rOUT, rCARRY
/* 14 +  2 */  strb rOUT, [rSCRATCH, #0]
/* 16      */

.endm


.macro bitbang_bit bit

/*  0 +  2 */  strb rOUT, [rGPIO, #0]
.ifeq \bit
/*  2 + 16 */  dither_bit0_16cycles
.else
/*  2 + 16 */  dither_bit_16cycles \bit
.endif
/* 18 +  2 */  strb rOUT, [rGPIO, #0]


/* 41 +  1 */  movs rOUT, #0
/* 42 +  2 */  strb rOUT, [rGPIO, #0]

/* 69 +  1 */  movs rOUT, #0xf0

.endm





