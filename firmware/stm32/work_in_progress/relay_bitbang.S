
#include <config.h>

#define rSCRATCH r0
#define rGPIO    r1
#define rOUT     r2
#define rCARRY   r3
#define rPREAD   r4

#define hiINBUF  r8
#define hiREMBUF r9
#define hiBUFEND r10

.include "m0delay.S"

    .cpu cortex-m0
    .fpu softvfp
    .section    .bss
//    .comm remainders,N_VALUES
//    .set VALUE_COUNT, N_VALUES
//    .set SEGMENT_COUNT, N_SEGMENTS
//    .set STRIP_COUNT, N_STRIPS
    .thumb
    .section    .text.bitbang,"ax",%progbits
    .balign 4
    .code   16

.macro push_r4_to_r11_14cycles

/*   0 +  5 */  push {r4-r7}
/*   5 +  1 */  mov r4, r8
/*   6 +  1 */  mov r5, r9
/*   7 +  1 */  mov r6, r10
/*   8 +  1 */  mov r7, r11
/*   9 +  5 */  push {r4-r7}

.endm

.macro pop_r4_to_r11_14cycles

/*   0 +  5 */  pop {r4-r7}
/*   5 +  1 */  mov r8, r4
/*   6 +  1 */  mov r9, r5
/*   7 +  1 */  mov r10, r6
/*   8 +  1 */  mov r11, r7
/*   9 +  5 */  pop {r4-r7}

.endm

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
 * B -+--------------|---. 
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

.ifeq \bit

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

.else

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

.endif

.endm

//

/* xx +  2 */  ldr rSCRATCH, =dma_cnt
/* xx +  2 */  ldr rSCRATCH, [rSCRATCH]
/* xx +  1 */  add rSCRATCH, rPREAD
/* xx +  1 */  cmp rSCRATCH, hiBUFEND
/* xx +  2 */  ldrb rSCRATCH, [rPREAD]
/* xx +  1 */  bne 0f
/* xx +  1 */  adds rPREAD, #1
/* xx +  1 */  cmps rPREAD, hiBUFEND
/* xx +  1 */  bne 1f
/* xx +  2 */  ldr rPREAD, =recv_buf
1:


0:
        ldrb rDATA, [rPREAD]

        cmp rPREAD, rBUFEND
        bne 1f
            ldr rPREAD, =recv_buf
        1:

        0:  ldr rSCRATCH, [rPCNT]
            subs rSCRATCH, rBUFEND, rSCRATCH
            cmp rSCRATCH, rPREAD
            beq 0b

        ldrb rSCRATCH, [rPREAD]
        lsls rSCRATCH, #8
        orrs rDATA, rSCRATCH

        adds rPREAD, #1
        cmp rPREAD, rBUFEND
        bne 1f
            ldr rPREAD, =recv_buf
        1:

        cmp rDATA, rFF00
            bgt frame_end

        strh rDATA, [rNEXT, rI]
        adds rI, #2

        cmp rI, rFRAMESZ
            blt read_loop

///


.macro bitbang_bit bit

/*  0 +  2 */  strb rOUT, [rGPIO, #0]
/*  2 + 16 */  dither_bit_16cycles \bit
/* 18 +  2 */  strb rOUT, [rGPIO, #0]


/* 41 +  1 */  movs rOUT, #0
/* 42 +  2 */  strb rOUT, [rGPIO, #0]

/* 69 +  1 */  movs rOUT, #0xf0

.endm


    .global SysTick_Handler
    .thumb_func
    .type   SysTick_Handler, %function

SysTick_Handler:

/*   0 +  2 */  ldr rSCRATCH, =GPIOA_ODR
/*   2 +  2 */  ldr rSCRATCH, =cur
/*   4 +  2 */  ldr rSCRATCH, [rSCRATCH]
/*   6 +  2 */  ldr rGPIO, =GPIOA_ODR

    .global bitbang
    .code   16
    .thumb_func
    .type   bitbang, %function
    .syntax unified
    .thumb

bitbang:

/*   8 + 14 */  push_r4_to_r11_14cycles
/*  22 +  1 */  subs rSCRATCH, #8
/*  23 +  1 */  mov hiINBUF, rSCRATCH
/*  24 +  2 */  ldr rSCRATCH, =rembuf -8
/*  26 +  1 */  mov hiREMBUF, rSCRATCH

//...

bitbang_bit 0
bitbang_bit 1
bitbang_bit 2
bitbang_bit 3
bitbang_bit 4
bitbang_bit 5
bitbang_bit 6
bitbang_bit 7

//...

/*  xx + 14 */  pop_r4_to_r11_14cycles
/*  xx +  2?*/  bx lr

    .size   bitbang, .-bitbang






