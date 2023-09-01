
#include "bitbang16_5strip.h"

#if N_STRIPS != 5
#error bad number of strips
#endif

#define BIT0_ADJ (0)
#define BIT1_ADJ (7)
#define BIT2_ADJ (2)
#define BIT3_ADJ (5)
#define BIT4_ADJ (1)
#define BIT5_ADJ (6)
#define BIT6_ADJ (3)
#define BIT7_ADJ (4)

.include "m0delay.S"
    .cpu cortex-m0
    .fpu softvfp
    .section    .bss
    .comm remainders,N_VALUES
    .set VALUE_COUNT, N_VALUES
    .set SEGMENT_COUNT, N_SEGMENTS
    .set STRIP_COUNT, N_STRIPS
    .thumb
    .section    .text.bitbang,"ax",%progbits
    .balign 4
    .code   16

    .global SysTick_Handler
    .thumb_func
    .type   SysTick_Handler, %function
    .syntax unified
    .thumb

SysTick_Handler:

ldr r0, =cur
ldr r0, [r0]

ldr r1, =GPIOA_ODR

ldr r2, =pulse_width8x
ldrh r2, [r2]

    .global bitbang16
    .code   16
    .thumb_func
    .type   bitbang16, %function
    .syntax unified
    .thumb


bitbang16:
/* push r4-r11 */

push {r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
push {r4-r7}

bitbang_start:

#define rGPIO_OUT  r6

#define rBUF_POINTER_HALF r8
#define rREM_DIFF_POINTER r9
#define rBUF_END_HALF     r10
#define rSTRIP_LEN        r11
#define rT1L              r12

lsrs r0, #1
mov rBUF_POINTER_HALF, r0

negs r0, r0
mov rREM_DIFF_POINTER, r0
ldr r0, =remainders
add rREM_DIFF_POINTER, r0

ldr r0, =(N_VALUES_PER_STRIP)
mov rSTRIP_LEN, r0

mov rBUF_END_HALF, rBUF_POINTER_HALF
add rBUF_END_HALF, rSTRIP_LEN

mov rGPIO_OUT, r1

movs r0, #33
lsls r0, #3
subs r2, r0
mov rT1L, r2

movs r3, #0
movs r4, #0
b bitbang_load

bitbang_bit_1_cycle13:

/* 13 +  2 */  strh r0, [rGPIO_OUT] // data

bitbang_load:
// r3: [  1  1  1  1  1 ] || 0
// r4: [ A0 B0 C0 D0 E0 ] || 0

/* 15 +  1 */  mov r7, rBUF_POINTER_HALF
/* 16 +  1 */  mov r5, rREM_DIFF_POINTER

/* 17 +  2 */  ldrh r0, [r7,r7]
/* 19 +  2 */  ldrb r2, [r5,r7]
/* 21 +  1 */  adds r0, r2
/* 22 +  2 */  strb r0, [r5,r7]               //          [ 0 ... A7 A6 A5 A4 A3 A2 A1 A0 {junk8} ]
/* 24 +  1 */  add r7, rSTRIP_LEN
/* 25 +  2 */  ldrh r1, [r7,r7]
/* 27 +  2 */  ldrb r2, [r5,r7]
/* 29 +  1 */  adds r1, r2
/* 30 +  2 */  strb r1, [r5,r7]
/* 32 +  1 */  lsls r1, r1, #16               //          [ B7 B6 B5 B4 B3 B2 B1 B0 {junk8} ... 0 ]
/* 33 +  2 */  strh r1, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r2, 2, BIT1_ADJ

bitbang_bit_0:
// r3:  0x1f || 0
// r4:  prev [ A0 B0 C0 D0 E0 ]

/*  0 +  2 */  strh r3, [rGPIO_OUT] // [ 1 1 1 1 1 ] || [ 0 0 0 0 0 ] (first iteration)
/*  2 +  1 */  nop
/*  3 +  1 */  add r7, rSTRIP_LEN
/*  4 +  2 */  ldrh r2, [r7,r7]
/*  6 +  2 */  ldrb r3, [r5,r7]
/*  8 +  1 */  adds r2, r3
/*  9 +  2 */  strb r2, [r5,r7]
/* 11 +  1 */  lsls r2, r2, #16               //          [ C7 C6 C5 C4 C3 C2 C1 C0 {junk8} ... 0 ]
/* 12 +  1 */  add r7, rSTRIP_LEN

/* 13 +  2 */  strh r4, [rGPIO_OUT] // data

/* 15 +  2 */  ldrh r3, [r7,r7]
/* 17 +  2 */  ldrb r4, [r5,r7]
/* 19 +  1 */  adds r3, r4
/* 20 +  2 */  strb r3, [r5,r7]
/* 22 +  1 */  lsls r3, r3, #16               //          [ D7 D6 D5 D4 D3 D2 D1 D0 {junk8} ... 0 ]

/* 23 +  1 */  add r7, rSTRIP_LEN
/* 24 +  2 */  ldrh r4, [r7, r7]
/* 26 +  1 */  add r5, r7
/* 27 +  2 */  ldrb r7, [r5]
/* 29 +  1 */  adds r4, r7
/* 30 +  2 */  strb r4, [r5]
/* 32 +  1 */  movs r5, #31                   //          [ 0                  ...  1  1  1  1  1 ]
/* 33 +  2 */  strh r3, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r7, 2, BIT0_ADJ

bitbang_bit_7:

//   r0:  [ A6 A5 A4 A3 A2 A1 A0 {junk8} ... 0 ]
//   r1:  [ 0 ... B6 B5 B4 B3 B2 B1 B0 {junk8} ]
//   r2:  [ C6 C5 C4 C3 C2 C1 C0 {junk8} ... 0 ]
//   r3:  [ D6 D5 D4 D3 D2 D1 D0 {junk8} ... 0 ]
//   r4:  [ 0 ... E6 E5 E4 E3 E2 E1 E0 {junk8} ]
//   r5:  ?
//   r6:  rGPIO_OUT
//   r7:  ?
//

/*  0 +  2 */  strh r5, [rGPIO_OUT] // 0x1f
/*  2 +  1 */  lsls r4, r4, #16               //          [ E7 E6 E5 E4 E3 E2 E1 E0 {junk8} ... 0 ]
/*  3 +  1 */  lsls r0, r0, #17               //  [ A7 ]  [ A6 A5 A4 A3 A2 A1 A0 {junk8}   ... 0 ]
/*  4 +  1 */  adcs r0, r0                    //  [ A6 ]  [ A5 A4 A3 A2 A1 A0 {junk8}     ... A7 ]
/*  5 +  1 */  adcs r1, r1                    //  [ B7 ]  [ B6 B5 B4 B3 B2 B1 B0 {junk8}...   A6 ]
/*  6 +  1 */  adcs r0, r0                    //  [ A5 ]  [ A4 A3 A2 A1 A0 {junk8}     ... A7 B7 ]
/*  7 +  1 */  adcs r2, r2                    //  [ C7 ]  [ C6 C5 C4 C3 C2 C1 C0 {junk8}...   A5 ]
/*  8 +  1 */  adcs r0, r0                    //  [ A4 ]  [ A3 A2 A1 A0 {junk8}     ... A7 B7 C7 ]
/*  9 +  1 */  adcs r3, r3                    //  [ D7 ]  [ D6 D5 D4 D3 D2 D1 D0 {junk8}...   A4 ]
/* 10 +  1 */  adcs r0, r0                    //  [ A3 ]  [ A2 A1 A0 {junk8}     ... A7 B7 C7 D7 ]
/* 11 +  1 */  adcs r4, r4                    //  [ E7 ]  [ E6 E5 E4 E3 E2 E1 E0 {junk8}...   A3 ]
/* 12 +  1 */  adcs r0, r0                    //  [ A2 ]  [ A1 A0 {junk8}     ... A7 B7 C7 D7 E7 ]
/* 13 +  2 */  strh r0, [rGPIO_OUT] // most significant data bits
/* 15 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0                             ... A2 ]
/* 16 +  1 */  bics r0, r5                    //  [  0 ]  [ A1 A0 {junk8}                 ...  0 ]
/* 17 +  1 */  adcs r1, r1                    //  [ B6 ]  [ B5 B4 B3 B2 B1 B0 {junk8}...   A6  0 ]
/* 18 +  1 */  adcs r1, r1                    //  [ B5 ]  [ B4 B3 B2 B1 B0 {junk8}...   A6  0 B6 ]
/* 19 +  1 */  adcs r2, r2                    //  [ C6 ]  [ C5 C4 C3 C2 C1 C0 {junk8}...   A5 B5 ]
/* 20 +  1 */  adcs r1, r1                    //  [ B4 ]  [ B3 B2 B1 B0 {junk8}...   A6  0 B6 C6 ]
/* 21 +  1 */  adcs r3, r3                    //  [ D6 ]  [ D5 D4 D3 D2 D1 D0 {junk8}...   A4 B4 ]
/* 22 +  1 */  adcs r1, r1                    //  [ B3 ]  [ B2 B1 B0 {junk8}...   A6  0 B6 C6 D6 ]
/* 23 +  1 */  adcs r4, r4                    //  [ E6 ]  [ E5 E4 E3 E2 E1 E0 {junk8}...   A3 B3 ]
/* 24 +  1 */  adcs r1, r1                    //  [ B2 ]  [ B1 B0 {junk8}...   A6  0 B6 C6 D6 E6 ]
/* 25 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0                          ... A2 B2 ]
/* 26 +  1 */  ands r5, r1                    //  [  0 ]  [ 0                    ... B6 C6 D6 E6 ]
/* 27 +  1 */  add  r1, r5                    //          [ B1 B0 {junk8} ...  A6 B6 C6 D6 E6  0 ]
/* 28 +  1 */  lsrs r1, #1                    //  [  0 ]  [  0 B1 B0 {junk8} ...  A6 B6 C6 D6 E6 ]
/* 29 +  1 */  nop
/* 30 +  1 */  nop
/* 31 +  1 */  movs r5, #0                    //          [ 0                 ...  0  0  0  0  0 ]
/* 32 +  1 */  adds r0, #31                   //          [ A1 A0 {junk8}     ...  1  1  1  1  1 ]
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT7_ADJ

bitbang_bit_6:
/*  0 +  2 */  strh r0, [rGPIO_OUT] // 0x1f
/*  2 +  1 */  subs r0, #31                   //          [ A1 A0 {junk8} ...      0  0  0  0  0 ]
/*  3 +  1 */  movs r5, #31                   //  [  0 ]  [ 0 ...                  1  1  1  1  1 ]
/*  4 +  1 */  adds r2, r2                    //  [ C5 ]  [ C4 C3 C2 C1 C0 {junk8} ...  A5 B5  0 ]
/*  5 +  1 */  adcs r2, r2                    //  [ C4 ]  [ C3 C2 C1 C0 {junk8}...   A5 B5  0 C5 ]
/*  6 +  1 */  adcs r3, r3                    //  [ D5 ]  [ D4 D3 D2 D1 D0 {junk8} ...  A4 B4 C4 ]
/*  7 +  1 */  adcs r2, r2                    //  [ C3 ]  [ C2 C1 C0 {junk8} ...  A5 B5  0 C5 D5 ]
/*  8 +  1 */  adcs r4, r4                    //  [ E5 ]  [ E4 E3 E2 E1 E0 {junk8} ...  A3 B3 C3 ]
/*  9 +  1 */  adcs r2, r2                    //  [ C2 ]  [ C1 C0 {junk8} ...  A5 B5  0 C5 D5 E5 ]
/* 10 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0                       ... A2 B2 C2 ]
/* 11 +  1 */  adds r3, r3                    //  [ D4 ]  [ D3 D2 D1 D0 {junk8} ...  A4 B4 C4  0 ]
/* 12 +  1 */  adcs r3, r3                    //  [ D3 ]  [ D2 D1 D0 {junk8} ...  A4 B4 C4  0 D4 ]
/* 13 +  2 */  strh r1, [rGPIO_OUT] // bits 6
/* 15 +  1 */  adcs r4, r4                    //  [ E4 ]  [ E3 E2 E1 E0 {junk8} ...  A3 B3 C3 D3 ]
/* 16 +  1 */  adcs r3, r3                    //  [ D2 ]  [ D1 D0 {junk8} ...  A4 B4 C4  0 D4 E4 ]
/* 17 +  1 */  adcs r7, r7                    //  [  0 ]  [  0                   ... A2 B2 C2 D2 ]
/* 18 +  1 */  bics r1, r5                    //  [  0 ]  [  0 B1 B0 {junk8} ...               0 ]
/* 19 +  1 */  rors r4, r4, r5                //  [ E3 ]  [ E2 E1 E0 {junk8} ...  A3 B3 C3 D3 E3 ] r5 val coincidence!
/* 20 +  1 */  rors r0, r0, r5                //  [ A1 ]  [ A0 {junk8}                        A1 ] r5 val coincidence!
/* 21 +  1 */  adcs r4, r4                    //  [ E2 ]  [ E1 E0 {junk8} ...  A3 B3 C3 D3 E3  0 ]
/* 22 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0 ...                 A2 B2 C2 D2 E2 ]
/* 23 +  1 */  lsls r1, #2                    //  [ B1 ]  [ B0 {junk8} ...                     0 ]
/* 24 +  1 */  adcs r0, r0                    //  [ A0 ]  [ {junk8} 0 ...                   A1 B1 ]
/* 25 +  1 */  adcs r1, r1                    //  [ B0 ]  [ {junk8} 0 ...                      A0 ]
/* 26 +  1 */  adcs r1, r1                    //  [ xx ]  [ {junk7} 0 ...                   A0 B0 ]
/* 27 +  1 */  adds r2, r2                    //  [ C1 ]  [ C0 {junk8} ...   A5 B5  0 C5 D5 E5  0 ]
/* 28 +  1 */  adcs r0, r0                    //  [ xx ]  [ {junk7} ...                  A1 B1 C1 ]
/* 29 +  1 */  nop
/* 30 +  1 */  lsls r7, #16                   //  [  0 ]  [ ... A2 B2 C2 D2 E2 ...          16x 0]
/* 31 +  1 */  adds r7, #31                   //  [  0 ]  [ ... A2 B2 C2 D2 E2 ... 1  1  1  1  1 ]
/* 32 +  1 */  movs r5, #0                    //  [  0 ]  [ 0 ...                                ]
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT6_ADJ

bitbang_bit_5:
 
// r0:  [ 0 ...                A1 B1 C1 ]
// r1:  [ ... A0 B0 C0 .. 1  1  1  1  1 ]
// r2:  [ 0 ... A5 B5 C5 D5 E5  0  0  0 ]
// r3:  [ D1 D0 ...   A4 B4 C4  0 D4 E4 ]
// r4:  [ E1 E0 ...   A3 B3 C3 D3 E3  0 ]
// r5:  [ 0          ...  1  1  1  1  1 ]
// r6:  rGPIO_OUT
// r7:  [ 0          ... A2 B2 C2 D2 E2 ]

/*  0 +  2 */  strh r7, [rGPIO_OUT] // 0x1f
/*  2 +  1 */  movs r5, #31
/*  3 +  1 */  ands r5, r2                    //  [  0 ]  [ 0 ...                     C5 D5 E5  0 ]
/*  4 +  1 */  add  r2, r5                    //  [  0 ]  [ C0 {junk8} ...   A5 B5 C5 D5 E5  0  0 ]
/*  5 +  1 */  adds r2, r2                    //  [ C0 ]  [ {junk8} ...   A5 B5 C5 D5 E5  0  0  0 ]
/*  6 +  1 */  adcs r1, r1                    //  [ xx ]  [ {junk6} ...                  A0 B0 C0 ]
/*  7 +  1 */  lsrs r2, #3                    //  [  0 ]  [ 0 0 0 {junk8} ...      A5 B5 C5 D5 E5 ]
/*  8 +  1 */  adds r3, r3                    //  [ D1 ]  [ D0 {junk8} ...   A4 B4 C4  0 D4 E4  0 ]
/*  9 +  1 */  adcs r0, r0                    //  [ xx ]  [ {junk6} ...               A1 B1 C1 D1 ]
/* 10 +  1 */  adds r4, r4                    //  [ E1 ]  [ E0 {junk8} ...   A3 B3 C3 D3 E3  0  0 ]
/* 11 +  1 */  adcs r0, r0                    //  [ xx ]  [ {junk5} ...            A1 B1 C1 D1 E1 ]
/* 12 +  1 */  adds r3, r3                    //  [ D0 ]  [ {junk8} ...   A4 B4 C4  0 D4 E4  0  0 ]
/* 13 +  2 */  strh r2, [rGPIO_OUT] // bits 5
/* 15 +  1 */  adcs r1, r1                    //  [  0 ]  [ 0 ...                     A0 B0 C0 D0 ]
/* 16 +  1 */  adcs r4, r4                    //  [ E0 ]  [ {junk8} ...   A3 B3 C3 D3 E3  0  0  0 ]
/* 17 +  1 */  adcs r1, r1                    //  [  0 ]  [ 0 ...                  A0 B0 C0 D0 E0 ]
/* 18 +  1 */  movs r5, #31
/* 19 +  1 */  ands r5, r3                    //  [  0 ]  [ 0                       0 D4 E4  0  0 ]
/* 20 +  1 */  lsrs r7, #16                   //  [  0 ]  [ 0 ...                        A0 B0 C0 ]
/* 21 +  1 */  add  r3, r5                    //  [  0 ]  [ {junk8}   ... A4 B4 C4 D4 E4  0  0  0 ]
/* 22 +  1 */  lsrs r3, #3                    //  [  0 ]  [ 0 0 0 {junk8} ...      A4 B4 C4 D4 E4 ]
/* 23 +  1 */  lsrs r4, #3                    //  [  0 ]  [ 0 0 0 {junk8} ...      A3 B3 C3 D3 E3 ]
/* 24 +  1 */  movs r2, #31                   //  [  0 ]  [ 0 ...                   1  1  1  1  1 ]
/* 25 +  8 */  delay r5, 8
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT5_ADJ  // r5 = 0

bitbang_bit_4:

/*  0 +  2 */  strh r2, [rGPIO_OUT] // 0x1f
/*  2 + 11 */  delay r5, 11
/* 13 +  2 */  strh r3, [rGPIO_OUT] // bits 4
/* 15 + 18 */  delay r5, 18
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT4_ADJ

bitbang_bit_3:

/*  0 +  2 */  strh r2, [rGPIO_OUT] // 0x1f
/*  2 + 11 */  delay r5, 11
/* 13 +  2 */  strh r4, [rGPIO_OUT] // bits 3
/* 15 + 18 */  delay r5, 18
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT3_ADJ

bitbang_bit_2:

/*  0 +  2 */  strh r2, [rGPIO_OUT] // 0x1f
/*  2 + 11 */  delay r5, 11
/* 13 +  2 */  strh r7, [rGPIO_OUT] // bits 2
/* 15 + 17 */  delay r5, 17
/* 32 +  1 */  movs r3, #31
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT2_ADJ

bitbang_bit_1:

/*  0 +  2 */  strh r2, [rGPIO_OUT] // 0x1f
/*  2 +  1 */  mov r4, r1
/*  3 +  1 */  movs r7, #1
/*  4 +  1 */  add rBUF_POINTER_HALF, r7
/*  5 +  1 */  cmp rBUF_POINTER_HALF, rBUF_END_HALF
/*  6 +1|3 */  beq 1f
/*  7 +  6 */  b bitbang_bit_1_cycle13
1:
/*  9 +  4 */  delay r5, 4
/* 13 +  2 */  strh r0, [rGPIO_OUT] // bits 1
/* 15 + 18 */  delay r5, 18
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
/* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT1_ADJ

bitbang_bit_0_epilogue:

/*  0 +  2 */  strh r2, [rGPIO_OUT] // 0x1f
/*  2 + 11 */  delay r5, 11
/* 13 +  2 */  strh r4, [rGPIO_OUT] // bits 0
/* 15 + 18 */  delay r5, 18
/* 33 +  2 */  strh r5, [rGPIO_OUT] // 0
///* 35 +  X */  delay_reg_div8 rT1L, r5, 2, BIT0_ADJ

bitbang_end:

/* pop r4-r11 */
pop {r4-r7}
mov r8, r4
mov r9, r5
mov r10, r6
mov r11, r7
pop {r4-r7}
bx lr
    .size   bitbang16, .-bitbang16

