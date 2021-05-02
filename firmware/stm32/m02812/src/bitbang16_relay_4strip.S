
#include <bitbang16_relay_4strip.h>

/* scatter gather bits: (bit-matrix transpose)
 *
 *  A[7-0] [ A7 A6 A5 A4 A3 A2 A1 A0 ]
 *  B[7-0] [ B7 B6 B5 B4 B3 B2 B1 B0 ]
 *  C[7-0] [ C7 C6 C5 C4 C3 C2 C1 C0 ]
 *  D[7-0] [ D7 D6 D5 D4 D3 D2 D1 D0 ]
 *
 *                ||
 *               \||/
 *                \/
 *
 *  [A-D]7 [  0  0  0  0 A7 B7 C7 D7 ]
 *  [A-D]6 [  0  0  0  0 A6 B6 C6 D6 ]
 *  [A-D]5 [  0  0  0  0 A5 B5 C5 D5 ]
 *  [A-D]4 [  0  0  0  0 A4 B4 C4 D4 ]
 *  [A-D]3 [  0  0  0  0 A3 B3 C3 D3 ]
 *  [A-D]2 [  0  0  0  0 A2 B2 C2 D2 ]
 *  [A-D]1 [  0  0  0  0 A1 B1 C1 D1 ]
 *  [A-D]0 [  0  0  0  0 A0 B0 C0 D0 ]
 *
 */

/*
 *         |--- 1 bit ----|
 *
 *         |---T1H---|-T1L|
 *
 *         |-T0H|---T0L---|
 *           A    B    C
 *         |----|----|----|
 *.....     ____..1...     ____
 *    |    |    |    |    |
 *    |    |    |    |    |
 *    |    |    |    |    |
 *....|____|    |.0..|____|
 *
 *         |1/8 iteration-|
 *         | 18 | 24 | 18 | x 8 -> 1 loop iteration
 *         |  60 cycles   | x 8 - (+/-) 8 x 5 ~= 440 cycles for computation
 *
 */

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
	.section	.bss
	.comm remainders,N_VALUES
	.set VALUE_COUNT, N_VALUES
	.set SEGMENT_COUNT, N_SEGMENTS
	.set STRIP_COUNT, N_STRIPS
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.balign	4
	.code	16

	.global	SysTick_Handler
	.thumb_func
	.type	SysTick_Handler, %function

SysTick_Handler:
ldr r1, =GPIOA_ODR

ldr r2, =pulse_width8x
ldrh r2, [r2]

ldr r0, =cur
ldr r0, [r0]

	.global	bitbang16
	.code	16
	.thumb_func
	.type	bitbang16, %function
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

mov r10, r0 // buffer
movs r6, r1  // GPIO_OUT

movs r0, #0
push {r0}  // current_segment(=0)

ldr r1, =remainders
mov r12, r1

subs r2, #4*42
subs r2, #4*42
mov r9, r2             /* r9  <-  8x T1L in cycles */

movs r1, r0
movs r2, r0
mov r11, r0
mov r8, r0

bitbang_loop:

bitbang_bit_2_cycle_10: /* except for iteration 0, in which we only load bitbang data */

/*     (n+1)'th iteration ||  first iteration
 *
 * r0: [A-D]2 (done)      ||  0b00000000
 * r1: [A-D]1 (done)      ||  0b00000000
 * r2: [A-D]0 (done)      ||  0b00000000
 * r6: &GPIO_OUT
 * r8: 0b00001111         ||  0b00000000
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_current_edge_of_first_strip
 * r12: &remainders[first_byte_of_current_edge_of_first_strip]
 */

/* 10 +  1 */  movs r4, #N_VALUES_PER_STRIP
/* 11 +  1 */  adds r4, r4
/* 12 +  1 */  mov r7, r10
/* 13 +  1 */  mov r5, r12
/* 14 +  1 */  nop
/* 15 +  1 */  add r5, r11

/*     (n+1)'th iteration ||  first iteration */
/* r0: [A-D]2 (done)      ||  0b00000000
 * r1: [A-D]1 (done)      ||  0b00000000
 * r2: [A-D]0 (done)      ||  0b00000000
 * r3: free
 * r4: #N_VALUES_PER_STRIP*2
 * r5: &remainders[current_byte_of_first_strip]
 * r6: &GPIO_OUT
 * r7: &buffer[current_byte_of_first_strip]
 * r8: 0b00001111         ||  0b00000000
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_current_edge_of_first_strip
 * r12: &remainders[first_byte_of_current_edge_of_first_strip]
 */

/* 16 +  2 */  ldrh r3, [r7] /* A -> r3 -> r0 */
/* phase B (T1H-T0H) */
/* 18 +  2 */  strh r0, [r6, #0]
/* 20 +  1 */  adds r7, r4

/*     (n+1)'th iteration ||  first iteration */
/* r0: free
 * r1: [A-D]1 (done)      ||  0b00000000
 * r2: [A-D]0 (done)      ||  0b00000000
 * r3:           pre-dither A[15-0]
 * r4: #N_VALUES_PER_STRIP*2
 * r5: &remainders[current_byte_of_first_strip]
 * r6: &GPIO_OUT
 * r7: &buffer[current_byte_of_first_strip+1*N_VALUES_PER_STRIP]
 * r8: 0b00001111         ||  0b00000000
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_current_edge_of_first_strip
 * r12: &remainders[first_byte_of_current_edge_of_first_strip]
 */

/* FREE: r0, r9 / ZERO OUT: r0 */
/* 21 + 21 */  delay r0, 21
/* phase C (T1L) */
/* 42 +  2 */  strh r0, [r6, #0] // 0x00
/* 44 + 15 */  delay_reg_div8 r9, r0, (18-15), BIT2_ADJ  // delay r0, 15
/* 59 +  1 */  mov r0, r8

bitbang_bit_1: /* except for iteration 0, in which we only load bitbang data */

/*  0 +  2 */  strh r0, [r6, #0] // 0x0f
/*  2 +  2 */  ldrb r0, [r5]
/*  4 +  1 */  adds r0, r3
/*  5 +  2 */  strb r0, [r5]
/*  7 +  1 */  adds r5, #N_VALUES_PER_STRIP
/*  8 +  2 */  ldrh r3, [r7] /* B -> r3 -> r1 */
/* 10 +  1 */  adds r7, r4
/* 11 +  2 */  ldrb r4, [r5]
/* 13 +  1 */  adds r3, r4
/* 14 +  2 */  strb r3, [r5]
/* 16 +  2 */  ldrh r4, [r7] /* C -> r4 -> r2 */

/* phase B (T1H-T0H) */
/* 18 +  2 */  strh r1, [r6, #0]
/* 20 +  1 */  movs r1, r3

/*     (n+1)'th iteration ||  first iteration */
/* r0:              A[7-0]<<8 + junk&0xff
 * r1:              B[7-0]<<8 + junk&0xff
 * r2: [A-D]0 (done)      ||  0b00000000
 * r3:                 free
 * r4:          pre-dither C[15-0]
 * r5: &remainders[current_byte_of_first_strip+1*N_VALUES_PER_STRIP]
 * r6: &GPIO_OUT
 * r7: &buffer[current_byte_of_first_strip+2*N_VALUES_PER_STRIP]
 * r8: 0b00001111         ||  0b00000000
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_current_edge_of_first_strip
 * r12: &remainders[first_byte_of_current_edge_of_first_strip]
 */

/* FREE: r3, r9 / ZERO OUT: r3 */

/* 21 + 21 */  delay r3, 21
/* phase C (T1L) */
/* 42 +  2 */  strh r3, [r6, #0] // 0x00
/* 44 + 15 */  delay_reg_div8 r9, r3, (18-15), BIT1_ADJ  // delay r3, 15
/* 59 +  1 */  mov r3, r8

bitbang_bit_0: /* except for iteration 0, in which we only load bitbang data */

/* phase A (T0H) */

/*  0 +  2 */  strh r3, [r6, #0] // 0x0f
/*  2 +  1 */  adds r5, #N_VALUES_PER_STRIP
/*  3 +  2 */  ldrb r3, [r5]
/*  5 +  1 */  adds r4, r3
/*  6 +  2 */  strb r4, [r5]

/*  8 +  1 */  adds r7, #N_VALUES_PER_STRIP
/*  9 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 10 +  1 */  adds r5, #N_VALUES_PER_STRIP
/* 11 +  2 */  ldrh r7, [r7]
/* 13 +  2 */  ldrb r3, [r5]
/* 15 +  1 */  adds r3, r7
/* 16 +  2 */  strb r3, [r5]

/* phase B (T1H-T0H) */
/* 18 +  2 */  strh r2, [r6, #0]
/* 20 +  1 */  mov r2, r4

/*     (n+1)'th iteration ||  first iteration */
/* r0:              A[7-0]<<8 + junk&0xff
 * r1:              B[7-0]<<8 + junk&0xff
 * r2:              C[7-0]<<8 + junk&0xff
 * r3:              D[7-0]<<8 + junk&0xff
 * r6: &GPIO_OUT
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_current_edge_of_first_strip
 * r12: &remainders[first_byte_of_current_edge_of_first_strip]
 */

/* FREE: r4, r5, r8, r9 / ZERO OUT: r4 */
/* 21 + 21 */  delay r4, 21
/* phase C (T1L) */
/* 42 +  2 */  strh r4, [r6, #0] // 0x00
/* 44 + 15 */  delay_reg_div8 r9, r4, (18-15), BIT0_ADJ  // delay r4, 15
/* 59 +  1 */  movs r7, #15

/* 
 * r0:  0 ... A[7-0] ? ? ? ? ? ? ? ?
 * r1:  0 ... B[7-0] ? ? ? ? ? ? ? ?
 * r2:  0 ... C[7-0] ? ? ? ? ? ? ? ?
 * r3:  0 ... D[7-0] ? ? ? ? ? ? ? ?
 * r4:  0
 * r5:  
 * r6:  &GPIO_OUT
 * r7:  scratch
 * r8:  
 * r9:  
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_current_edge_of_first_strip
 * r12: &remainders[first_byte_of_current_edge_of_first_strip]
 */

bitbang_bit_7:

/* phase A (T0H) */

/*  0 +  2 */  strh r7, [r6, #0] // 0x0f
/*  2 +  1 */  movs r5, #1
/*  3 +  1 */  add r10, r5  // pointer to 
/*  4 +  1 */  add r10, r5  //
/*  5 +  1 */  add r11, r5  //

bitbang_bit_7_cycle_6:
                                              //  carry   [           register            ]
/*  6 +  1 */  lsls r3, r3, #16               //          [ D7 D6 D5 D4 D3 D2 D1 D0 ... 0 ]
/*  7 +  1 */  lsls r2, r2, #16               //          [ C7 C6 C5 C4 C3 C2 C1 C0 ... 0 ]
/*  8 +  1 */  lsls r1, r1, #16               //          [ B7 B6 B5 B4 B3 B2 B1 B0 ... 0 ]
/*  9 +  1 */  lsls r0, r0, #17               //  [ A7 ]  [ A6 A5 A4 A3 A2 A1 A0    ... 0 ]
/* 10 +  1 */  adcs r0, r0                    //  [ A6 ]  [ A5 A4 A3 A2 A1 A0      ... A7 ]
/* 11 +  1 */  adcs r1, r1                    //  [ B7 ]  [ B6 B5 B4 B3 B2 B1 B0 ...   A6 ]
/* 12 +  1 */  adcs r0, r0                    //  [ A5 ]  [ A4 A3 A2 A1 A0      ... A7 B7 ]
/* 13 +  1 */  adcs r2, r2                    //  [ C7 ]  [ C6 C5 C4 C3 C2 C1 C0 ...   A5 ]
/* 14 +  1 */  adcs r0, r0                    //  [ A4 ]  [ A3 A2 A1 A0      ... A7 B7 C7 ]
/* 15 +  1 */  adcs r3, r3                    //  [ D7 ]  [ D6 D5 D4 D3 D2 D1 D0 ...   A4 ]
/* 16 +  1 */  adcs r0, r0                    //  [ A3 ]  [ A2 A1 A0      ... A7 B7 C7 D7 ]
/* 17 +  1 */  adcs r4, r4                    //  [  0 ]  [                      ...   A3 ]
/* phase B (T1H-T0H) */
/* 18 +  2 */  strh r0, [r6, #0]              //          <  0 ... 0  0  0  0 A7 B7 C7 D7 >
/* 20 +  1 */  bics r0, r7                    //  [  0 ]  [ A2 A1 A0      ...  0  0  0  0 ]
/* FREE: r5, r7, r8, r9 / ZERO OUT: r7 */
/* 21 + 21 */  delay r7, 21
/* phase C (T1L) */
/* 42 +  2 */  strh r0, [r6, #0] // 0x00
/* 44 + 15 */  delay_reg_div8 r9, r7, (18-15), BIT7_ADJ  // delay r7, 15
/* 59 +  1 */  movs r7, #15
bitbang_bit_6:

/* phase A (T0H) */

/*  0 +  2 */  strh r7, [r6, #0] // 0x0f

/*  2 +  1 */  adds r1, r1                    //  [ B6 ]  [ B5 B4 B3 B2 B1 B0 ...   A6  0 ]
/*  3 +  1 */  adcs r1, r1                    //  [ B5 ]  [ B4 B3 B2 B1 B0 ...   A6  0 B6 ]
/*  4 +  1 */  adcs r2, r2                    //  [ C6 ]  [ C5 C4 C3 C2 C1 C0 ...   A5 B5 ]
/*  5 +  1 */  adcs r1, r1                    //  [ B4 ]  [ B3 B2 B1 B0 ...   A6  0 B6 C6 ]
/*  6 +  1 */  adcs r3, r3                    //  [ D6 ]  [ D5 D4 D3 D2 D1 D0 ...   A4 B4 ]
/*  7 +  1 */  adcs r1, r1                    //  [ B3 ]  [ B2 B1 B0 ...   A6  0 B6 C6 D6 ]
/*  8 +  1 */  adcs r4, r4                    //  [  0 ]  [                   ...   A3 B3 ]
/*  9 +  1 */  movs r5, #0x10                 //  [  0 ]  [           ...   1  0  0  0  0 ]
/* 10 +  1 */  ands r5, r1                    //  [  0 ]  [           ...  A6  0  0  0  0 ]
/* 11 +  1 */  lsrs r5, #1                    //  [  0 ]  [           ...   0 A6  0  0  0 ]
/* 12 +  1 */  subs r1, r5                    //  [  0 ]  [ B2 B1 B0 ...    0 A6 B6 C6 D6 ]
/* 13 +  1 */  adds r2, r2                    //  [ C5 ]  [ C4 C3 C2 C1 C0 ...   A5 B5  0 ]
/* 14 +  1 */  adcs r2, r2                    //  [ C4 ]  [ C3 C2 C1 C0 ...   A5 B5  0 C5 ]
/* 15 +  1 */  adcs r3, r3                    //  [ D5 ]  [ D4 D3 D2 D1 D0 ...   A4 B4 C4 ]
/* 16 +  1 */  adcs r2, r2                    //  [ C3 ]  [ C2 C1 C0 ...   A5 B5  0 C5 D5 ]
/* 17 +  1 */  adcs r4, r4                    //  [  0 ]  [                ...   A3 B3 C3 ]
/* phase B (T1H-T0H) */
/* 18 +  2 */  strh r1, [r6, #0]
/* 20 +  1 */  bics r1, r7                    //  [  0 ]  [ B2 B1 B0      ...  0  0  0  0 ]
/* FREE: r5, r7, r8, r9 / ZERO OUT: r7 */
/* 21 + 21 */  delay r7, 21
/* phase C (T1L) */
/* 42 +  2 */  strh r1, [r6, #0] // 0x00
/* 44 + 15 */  delay_reg_div8 r9, r7, (18-15), BIT6_ADJ  // delay r7, 15
/* 59 +  1 */  movs r7, #15
bitbang_bit_5:

/* phase A (T0H) */

/*  0 +  2 */  strh r7, [r6, #0] // 0x0f
/*  2 +  1 */  movs r5, #0x18                 //  [  0 ]  [           ...   1  1  0  0  0 ]
/*  3 +  1 */  ands r5, r2                    //  [  0 ]  [           ...  A5 B5  0  0  0 ]
/*  4 +  1 */  lsrs r5, #1                    //  [  0 ]  [           ...   0 A5 B5  0  0 ]
/*  5 +  1 */  subs r2, r5                    //  [  0 ]  [ C2 C1 C0 ...      A5 B5 C5 D5 ]
/*  6 +  1 */  adds r3, r3                    //  [ D4 ]  [ D3 D2 D1 D0 ...   A4 B4 C4  0 ]
/*  7 +  1 */  adcs r3, r3                    //  [ D3 ]  [ D2 D1 D0 ...   A4 B4 C4  0 D4 ]
/*  8 +  1 */  adcs r4, r4                    //  [  0 ]  [             ...   A3 B3 C3 D3 ]
/*  9 +  1 */  movs r5, #0x1c                 //  [  0 ]  [           ...   1  1  1  0  0 ]
/* 10 +  1 */  ands r5, r3                    //  [  0 ]  [           ...  A4 B4 C4  0  0 ]
/* 11 +  1 */  lsrs r5, #1                    //  [  0 ]  [           ...   0 A4 B4 C4  0 ]
/* 12 +  1 */  subs r3, r5                    //  [  0 ]  [ D2 D1 D0 ...      A4 B4 C4 D4 ]

/* 13 +  5 */  delay r5, 5

/* 18 +  2 */  strh r2, [r6, #0]
/* phase B (T1H-T0H) */
/* 20 +  1 */  bics r2, r7                    //  [  ? ]  [ C2 C1 C0 ...       0  0  0  0 ]
/* FREE: r5, r7, r8, r9 / ZERO OUT: r7 */
/* 21 + 21 */  delay r7, 21
/* phase C (T1L) */
/* 42 +  2 */  strh r2, [r6, #0] // 0x00
/* 44 + 15 */  delay_reg_div8 r9, r7, (18-15), BIT5_ADJ  // delay r7, 15
/* 59 +  1 */  movs r7, #15
bitbang_bit_4:

/* phase A (T0H) */

/*  0 +  2 */  strh r7, [r6, #0] // 0x0f
/*  2 +  1 */  adcs r0, r0                    //  [ A2 ]  [ A1 A0      ...              0 ]
/*  3 +  1 */  adcs r0, r0                    //  [ A1 ]  [ A0         ...             A2 ]
/*  4 +  1 */  adcs r1, r1                    //  [ B2 ]  [ B1 B0      ...             A1 ]
/*  5 +  1 */  adcs r0, r0                    //  [ A0 ]  [  ?         ...          A2 B2 ]
/*  6 +  1 */  adcs r2, r2                    //  [ C2 ]  [ C1 C0      ...             A0 ]
/*  7 +  1 */  adcs r0, r0                    //  [  ? ]  [  ?         ...       A2 B2 C2 ]

/*  8 + 10 */  delay r5, 10

/* phase B (T1H-T0H) */
/* 18 +  2 */  strh r3, [r6, #0]
/* 20 +  1 */  bics r3, r7                    //  [  ? ]  [ D2 D1 D0 ...       0  0  0  0 ]
/* FREE: r5, r7, r8, r9 / ZERO OUT: r7 */
/* 21 + 21 */  delay r7, 21
/* phase C (T1L) */
/* 42 +  2 */  strh r3, [r6, #0] // 0x0f
/* 44 + 15 */  delay_reg_div8 r9, r7, (18-15), BIT5_ADJ  // delay r7, 15
/* 59 +  1 */  movs r7, #15
bitbang_bit_3:

/* phase A (T0H) */

/*  0 +  2 */  strh r7, [r6, #0] // 0x0f
/*  2 +  1 */  adcs r3, r3                    //  [ D2 ]  [ D1 D0      ...              0 ]
/*  3 +  1 */  adcs r0, r0                    //  [  ? ]  [  ?         ...    A2 B2 C2 D2 ]
/*  4 +  1 */  movs r5, #0x1f
/*  5 +  1 */  rors r1, r5                    //  [ B0 ]  [ B0 ...                  A1 B1 ]
/*  6 +  1 */  adcs r2, r2                    //  [ C1 ]  [ C0 ...                  A0 B0 ]
/*  7 +  1 */  adcs r1, r1                    //  [ B0 ]  [  ? ...               A1 B1 C1 ]
/*  8 +  1 */  adcs r3, r3                    //  [ D1 ]  [ D0 ...                      0 ]
/*  9 +  1 */  adcs r1, r1                    //  [  ? ]  [  ? ...            A1 B1 C1 D1 ]
/* 10 +  1 */  rors r2, r5                    //  [ C0 ]  [ C0 ...               A0 B0 C0 ]
/* 11 +  1 */  adcs r3, r3                    //  [ D0 ]  [  ? ...                      0 ]
/* 12 +  1 */  adcs r2, r2                    //  [ C0 ]  [  ? ...            A0 B0 C0 D0 ]
/* 13 +  1 */  movs r5, #15
/* 14 +  1 */  mov  r8, r5
/* 15 +  1 */  nop
/* 16 +  1 */  nop
/* 17 +  1 */  nop
/* phase B (T1H-T0H) */
/* 18 +  2 */  strh r4, [r6, #0]
/* FREE: r5, r7, r4, r9 / ZERO OUT: r7 */
/* 20 + 22 */  delay r7, 22
/* phase C (T1L) */
/* 42 +  2 */  strh r7, [r6, #0] // 0x00

/* r0: [A-D]2 (done)
 * r1: [A-D]1 (done)
 * r2: [A-D]0 (done)
 * r6: &GPIO_OUT
 * r8: 0b00001111
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_current_edge_of_first_strip
 * r12: &remainders[first_byte_of_current_edge_of_first_strip]
 */

/* 44 + 15 */  delay_reg_div8 r9, r7, (18-15), BIT3_ADJ  // delay r7, 15
/* 59 +  1 */  movs r4, #N_VALUES_PER_SEGMENT
bitbang_bit_2:
/*  0 +  2 */  strh r5, [r6, #0] // 0xff
/*  2 +  1 */  cmp r4, r11
/*  3 +  1 | 3 */ beq 1f
/*  4 + 3+(3?) */ b bitbang_bit_2_cycle_10
1:
/*  6 +  1 */  movs r4, #0
/*  7 +  1 */  mov r11, r4
/*  8 + 10 */  delay r7, 10
/* 18 +  2 */  strh r0, [r6, #0]
/* FREE: r0, r5, r7, r4, r9 / ZERO OUT: r7 */
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 15 */  delay_reg_div8 r9, r7, (18-15), BIT2_ADJ  // delay r7, 15
/* 59 +  1 */  movs r7, #15
bitbang_bit_1_epilogue:
/*  0 +  2 */  strh r7, [r6, #0] // 0x0f
/*  2 +  2 */  pop {r5}                  // {routing_i, routing_table}
/*  4 +  2 */  ldr r7, =routing_table
/*  6 +  2 */  ldr r7, [r7]
/*  8 +  1 */  adds r4, r7, r5
/*  9 +  1 */  adds r5, #1
/* 10 +  2 */  push {r5}
/* 12 +  1 */  movs r7, #N_VALUES_PER_SEGMENT
/* 13 +  1 */  add r12, r7
/* 14 +  4 */  delay r7, 4
/* 18 +  2 */  strh r1, [r6, #0] // [A-D]1
/* FREE: r0, r5, r7, r1, r9 / ZERO OUT: r7 */
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 15 */  delay_reg_div8 r9, r7, (18-15), BIT1_ADJ  // delay r7, 15
/* 59 +  1 */  movs r7, #15
bitbang_bit_0_epilogue:
/*  0 +  2 */  strh r7, [r6, #0] // 0x0f
/*  2 +  2 */  ldrb r0, [r4]
/*  4 +  1 */  adds r4, #(N_SEGMENTS-1)
/*  5 +  2 */  ldrb r1, [r4]
/*  7 +  1 */  adds r4, #(N_SEGMENTS-1)
/*  8 +  2 */  ldrb r7, [r4]
/* 10 +  1 */  adds r4, #(N_SEGMENTS-1)
/* 11 +  2 */  ldrb r3, [r4]
/* 13 +  1 */  subs r5, #(N_SEGMENTS)
/* 14 +  1 */  lsls r0, #8
/* 15 +  1 */  lsls r1, #8
/* 16 +  1 */  lsls r7, #8
/* 17 +  1 */  lsls r3, #8
/* 18 +  2 */  strh r2, [r6, #0] // [A-D]0 (done)
/* 20 +  1 */  movs r2, r7
/* FREE: r7, r9 / ZERO OUT: r7 */
/* 21 + 21 */  delay r7, 21
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 14 */  delay_reg_div8 r9, r4, (18-14), BIT0_ADJ  // delay r4, 14
/* 58 +  1 */  tst r5, r5
/* 59 + 1|3*/  beq 1f
/* 60 + 3+3*/ b bitbang_bit_7_cycle_6
1:
pop {r1}
bitbang_end:

/* pop r4-r11 */
pop {r4-r7}
mov r8, r4
mov r9, r5
mov r10, r6
mov r11, r7
pop {r4-r7}
bx lr
	.size	bitbang16, .-bitbang16

