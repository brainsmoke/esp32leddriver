
#include <bitbang.h>

/* scatter gather bits: (bit-matrix transpose)
 *
 *  A[7-0] [ A7 A6 A5 A4 A3 A2 A1 A0 ]
 *  B[7-0] [ B7 B6 B5 B4 B3 B2 B1 B0 ]
 *  C[7-0] [ C7 C6 C5 C4 C3 C2 C1 C0 ]
 *  D[7-0] [ D7 D6 D5 D4 D3 D2 D1 D0 ]
 *  E[7-0] [ E7 E6 E5 E4 E3 E2 E1 E0 ]
 *  F[7-0] [ F7 F6 F5 F4 F3 F2 F1 F0 ]
 *  G[7-0] [ G7 G6 G5 G4 G3 G2 G1 G0 ]
 *  H[7-0] [ H7 H6 H5 H4 H3 H2 H1 H0 ]
 *
 *                ||
 *               \||/
 *                \/
 *
 *  [A-H]7 [ A7 B7 C7 D7 E7 F7 G7 H7 ]
 *  [A-H]6 [ A6 B6 C6 D6 E6 F6 G6 H6 ]
 *  [A-H]5 [ A5 B5 C5 D5 E5 F5 G5 H5 ]
 *  [A-H]4 [ A4 B4 C4 D4 E4 F4 G4 H4 ]
 *  [A-H]3 [ A3 B3 C3 D3 E3 F3 G3 H3 ]
 *  [A-H]2 [ A2 B2 C2 D2 E2 F2 G2 H2 ]
 *  [A-H]1 [ A1 B1 C1 D1 E1 F1 G1 H1 ]
 *  [A-H]0 [ A0 B0 C0 D0 E0 F0 G0 H0 ]
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
.include "m0delay.S"

	.cpu cortex-m0
	.fpu softvfp
	.section	.bss
	.comm gamma_map,512
	.comm remainders,N_VALUES
	.set VALUE_COUNT, N_VALUES
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.align	2
	.global	bitbang
	.code	16
	.thumb_func
	.type	bitbang, %function
bitbang:
	.syntax unified
	.thumb

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
movs r1, r0
movs r5, r0
mov r9, r0
mov r11, r0

bitbang_loop:

bitbang_bit_2_cycle_14: /* except for iteration 0, in which we only load bitbang data */

/*     (n+1)'th iteration ||  first iteration */

/* r0: [A-H]2 (done)     ||  0
 * r1: 0xff              ||  0
 * r2:
 * r3:
 * r4:
 * r5: [A-H]0 (done)     ||  0
 * r6: &GPIO_OUT
 * r7:
 * r8:
 * r9: [A-H]1 (done)     ||  0
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_first_strip
 */

/* 14 +  1 */  mov r7, r10
/* 15 +  1 */  mov r10, r5
/* 16 +  2 */  ldrb r2, [r7]                                    /* moved up from below */
/* 18 +  2 */  strh r0, [r6, #0]               // [A-H]2 || 0
/* 20 +  2 */  ldr r5, =remainders                              /* flash load, wait state? */
/* 21 +  1 */  add r5, r11
/* 23 +  2 */  ldr r4, =gamma_map                               /* flash load, wait state? */

/*     (n+1)'th iteration ||  first iteration */

/* r0:
 * r1: 0xff               ||  0
 * r2: (uint8_t)buffer[current_byte_of_first_strip]
 * r3:
 * r4: gamma_map
 * r5: &remainders[current_byte_of_first_strip]
 * r6: &GPIO_OUT
 * r7: &buffer[current_byte_of_first_strip]
 * r8:
 * r9: [A-H]1 (done)      ||  0
 * r10: [A-H]0 (done)     ||  0
 * r11: current_byte_of_first_strip
 */

/* load A[7-0] */

/* 25 +  1 */  mov r8, r1

/* 26 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 27 +  1 */  lsls r2, r2, #1
/* 28 +  2 */  ldrh r2, [r2, r4]
/* 30 +  2 */  ldrb r3, [r5]
/* 32 +  1 */  adds r2, r3
/* 33 +  2 */  strb r2, [r5]
/* 35 +  1 */  adds r5, #N_VALUES_PER_STRIP
/* 36 +  1 */  lsrs r0, r2, #8                 // r0: [ 0 ... A[7-0] ]

/* load B[7-0] */
/* 37 +  2 */  ldrb r2, [r7]
/* 39 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 40 +  1 */  lsls r2, r2, #1

/* 41 +  1 */  movs r3, #0
/* 42 +  2 */  strh r3, [r6, #0]               // 0

/* 44 +  2 */  ldrh r2, [r2, r4]
/* 46 +  2 */  ldrb r3, [r5]
/* 48 +  1 */  adds r2, r3
/* 49 +  2 */  strb r2, [r5]
/* 51 +  1 */  adds r5, #N_VALUES_PER_STRIP
/* 52 +  1 */  lsrs r1, r2, #8                 // r1: [ 0 ... B[7-0] ]

/* load C[7-0] */
/* 53 +  2 */  ldrb r2, [r7]
/* 55 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 56 +  1 */  lsls r2, r2, #1
/* 57 +  2 */  ldrh r2, [r2, r4]


/* 59 +  1 */  mov r3, r8

bitbang_bit_1: /* except for iteration 0, in which we only load bitbang data */
/*  0 +  2 */  strh r3, [r6, #0]               // 0xff || 0
/*  2 +  2 */  ldrb r3, [r5]
/*  4 +  1 */  adds r2, r3
/*  5 +  2 */  strb r2, [r5]
/*  7 +  1 */  adds r5, #N_VALUES_PER_STRIP
/*  8 +  1 */  lsrs r2, r2, #8
/*  9 +  1 */  lsls r2, r2, #8
/* 10 +  1 */  adds r0, r2                     // r0: [ 0 ... C[7-0] A[7-0] ]

/* load D[7-0] */
/* 11 +  2 */  ldrb r2, [r7]
/* 13 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 14 +  1 */  lsls r2, r2, #1
/* 15 +  2 */  ldrh r2, [r2, r4]

/* 17 +  1 */  mov r3, r9
/* 18 +  2 */  strh r3, [r6, #0]               // [A-H]1 || 0

/* 20 +  2 */  ldrb r3, [r5]
/* 22 +  1 */  adds r2, r3
/* 23 +  2 */  strb r2, [r5]
/* 25 +  1 */  adds r5, #N_VALUES_PER_STRIP
/* 26 +  1 */  lsrs r2, r2, #8
/* 27 +  1 */  lsls r2, r2, #8
/* 28 +  1 */  adds r1, r2                     // r1: [ 0 ... D[7-0] B[7-0] ]

/* load E[7-0] */
/* 29 +  2 */  ldrb r2, [r7]
/* 31 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 32 +  1 */  lsls r2, r2, #1
/* 33 +  2 */  ldrh r2, [r2, r4]
/* 35 +  2 */  ldrb r3, [r5]
/* 37 +  1 */  adds r2, r3
/* 38 +  2 */  strb r2, [r5]
/* 40 +  1 */  adds r5, #N_VALUES_PER_STRIP

/* 41 +  1 */  movs r3, #0
/* 42 +  2 */  strh r3, [r6, #0]               // 0

/* 44 +  1 */  lsls r2, r2, #16
/* 45 +  1 */  adds r0, r2                     // r0: [ E[7-0] [8-bits junk] C[7-0] A[7-0] ]

/* load F[7-0] */
/* 46 +  2 */  ldrb r2, [r7]
/* 48 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 49 +  1 */  lsls r2, r2, #1
/* 50 +  2 */  ldrh r2, [r2, r4]
/* 52 +  2 */  ldrb r3, [r5]
/* 54 +  1 */  adds r2, r3
/* 55 +  2 */  strb r2, [r5]
/* 57 +  1 */  adds r5, #N_VALUES_PER_STRIP
/* 58 +  1 */  lsrs r2, r2, #8

/* 59 +  1 */  mov r3, r8

bitbang_bit_0: /* except for iteration 0, in which we only load bitbang data */
/*  0 +  2 */  strh r3, [r6, #0]

/*  2 +  1 */  lsls r2, r2, #24
/*  3 +  1 */  mov r8, r2

/* load G[7-0] */
/*  4 +  2 */  ldrb r2, [r7]
/*  6 +  1 */  adds r7, #N_VALUES_PER_STRIP
/*  7 +  1 */  lsls r2, r2, #1
/*  8 +  2 */  ldrh r2, [r2, r4]
/* 10 +  2 */  ldrb r3, [r5]
/* 12 +  1 */  adds r2, r3
/* 13 +  2 */  strb r2, [r5]
/* 15 +  1 */  adds r5, #N_VALUES_PER_STRIP
/* 16 +  1 */  lsrs r2, r2, #8

/* 17 +  1 */  mov r3, r10
/* 18 +  2 */  strh r3, [r6, #0]              // [A-H]0 || 0

/* 20 +  1 */  lsls r2, r2, #24
/* 21 +  1 */  mov r9, r2

/* load H[7-0] */

/* 22 +  2 */  ldrb r2, [r7]
/* 24 +  1 */  adds r7, #N_VALUES_PER_STRIP
/* 25 +  1 */  lsls r2, r2, #1
/* 26 +  2 */  ldrh r2, [r2, r4]
/* 28 +  2 */  ldrb r3, [r5]
/* 30 +  1 */  adds r2, r3
/* 31 +  2 */  strb r2, [r5]
/* 33 +  1 */  adds r5, #N_VALUES_PER_STRIP
/* 34 +  1 */  lsls r2, r2, #16
/* 35 +  1 */  adds r1, r2                     // r1: [ H[7-0] [8-bits junk] D[7-0] B[7-0] ]


/* 36 +  1 */  movs r4, #N_VALUES_PER_STRIP
/* 37 +  1 */  lsls r4, r4, #3
/* 38 +  1 */  subs r7, r4
/* 39 +  1 */  mov r10, r7
/* 40 +  1 */  movs r5, #1
/* 41 +  1 */  movs r4, #0

/* r0:  [ E[7-0] [8-bits junk] C[7-0] A[7-0] ]
 * r1:  [ H[7-0] [8-bits junk] D[7-0] B[7-0] ]
 * r2:
 * r3:
 * r4:  0
 * r5:  1
 * r6:  &GPIO_OUT
 * r7:  &buffer[current_byte_of_first_strip]
 * r8:  F[7-0] ... 0
 * r9:  G[7-0] ... 0
 * r10:
 * r11: current_byte_of_first_strip
 */

/* phase C (T1L) bit0 */
                                             //  carry   [           register            ]

/* 42 +  2 */  strh r4, [r6, #0] // 0
/* 44 +  1 */  add r10, r5
/* 45 +  1 */  add r11, r5
/* 46 +  1 */  lsrs r2, r0, #8
/* 47 +  1 */  lsls r2, r2, #24               //          [ C7 C6 C5 C4 C3 C2 C1 C0 ... 0 ]
/* 48 +  1 */  lsrs r3, r1, #8
/* 49 +  1 */  lsls r3, r3, #24               //          [ D7 D6 D5 D4 D3 D2 D1 D0 ... 0 ]
/* 50 +  1 */  lsrs r4, r0, #24
/* 51 +  1 */  lsls r4, r4, #24               //          [ E7 E6 E5 E4 E3 E2 E1 E0 ... 0 ]
/* 52 +  1 */  lsrs r5, r1, #24
/* 53 +  1 */  lsls r5, r5, #24               //          [ H7 H6 H5 H4 H3 H2 H1 H0 ... 0 ]
/* 54 +  1 */  lsls r1, r1, #24               //          [ B7 B6 B5 B4 B3 B2 B1 B0 ... 0 ]
/* 55 +  1 */  movs r7, #255
/* 56 +  1 */  lsls r0, r0, #25               //  [ A7 ]  [ A6 A5 A4 A3 A2 A1 A0    ... 0 ]

/* C :  A7
 * r0:  A[6-0] ... 0
 * r1:  B[7-0] ... 0
 * r2:  C[7-0] ... 0
 * r3:  D[7-0] ... 0
 * r4:  E[7-0] ... 0
 * r5:  H[7-0] ... 0
 * r6:  &GPIO_OUT
 * r7:  scratch
 * r8:  F[7-0] ... 0
 * r9:  G[7-0] ... 0
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_first_strip
 */

/* 57 +  1 */  adcs r0, r0                    //  [ A6 ]  [ A5 A4 A3 A2 A1 A0      ... A7 ]
/* 58 +  1 */  adcs r1, r1                    //  [ B7 ]  [ B6 B5 B4 B3 B2 B1 B0 ...   A6 ]
/* 59 +  1 */  adcs r0, r0                    //  [ A5 ]  [ A4 A3 A2 A1 A0      ... A7 B7 ]

bitbang_bit_7:

/* phase A (T0H) */

/*  0 +  2 */  strh r7, [r6, #0] // 0xff

/*  2 +  1 */  adcs r2, r2                    //  [ C7 ]  [ C6 C5 C4 C3 C2 C1 C0 ...   A5 ]
/*  3 +  1 */  adcs r0, r0                    //  [ A4 ]  [ A3 A2 A1 A0      ... A7 B7 C7 ]
/*  4 +  1 */  adcs r3, r3                    //  [ D7 ]  [ D6 D5 D4 D3 D2 D1 D0 ...   A4 ]
/*  5 +  1 */  adcs r0, r0                    //  [ A3 ]  [ A2 A1 A0      ... A7 B7 C7 D7 ]
/*  6 +  1 */  adcs r4, r4                    //  [ E7 ]  [ E6 E5 E4 E3 E2 E1 E0 ...   A3 ]
/*  7 +  1 */  adcs r0, r0                    //  [ A2 ]  [ A1 A0      ... A7 B7 C7 D7 E7 ]
/*  8 +  1 */  mov r7, r8
/*  9 +  1 */  adcs r7, r7                    //  [ F7 ]  [ F6 F5 F4 F3 F2 F1 F0 ...   A2 ]
/* 10 +  1 */  mov r8, r7
/* 11 +  1 */  adcs r0, r0                    //  [ A1 ]  [ A0      ... A7 B7 C7 D7 E7 F7 ]
/* 12 +  1 */  mov r7, r9
/* 13 +  1 */  adcs r7, r7                    //  [ G7 ]  [ G6 G5 G4 G3 G2 G1 G0 ...   A1 ]
/* 14 +  1 */  mov r9, r7
/* 15 +  1 */  adcs r0, r0                    //  [ A0 ]  [ 0    ... A7 B7 C7 D7 E7 F7 G7 ]
/* 16 +  1 */  adcs r5, r5                    //  [ H7 ]  [ H6 H5 H4 H3 H2 H1 H0 ...   A0 ]
/* 17 +  1 */  adcs r0, r0                    //  [  0 ]  [ 0 ... A7 B7 C7 D7 E7 F7 G7 H7 ]

/* phase B (T1H-T0H) */

/* r0: [A-H]7 (done) */

/* 18 +  2 */  strh r0, [r6, #0] // most significant data bits

/* r0: free */

/* 20 +  1 */  adds r1, r1                    //  [ B6 ]  [ B5 B4 B3 B2 B1 B0 ...   A6  0 ]
/* 21 +  1 */  adcs r1, r1                    //  [ B5 ]  [ B4 B3 B2 B1 B0 ...   A6  0 B6 ]
/* 22 +  1 */  adcs r2, r2                    //  [ C6 ]  [ C5 C4 C3 C2 C1 C0 ...   A5 B5 ]
/* 23 +  1 */  adcs r1, r1                    //  [ B4 ]  [ B3 B2 B1 B0 ...   A6  0 B6 C6 ]
/* 24 +  1 */  adcs r3, r3                    //  [ D6 ]  [ D5 D4 D3 D2 D1 D0 ...   A4 B4 ]
/* 25 +  1 */  adcs r1, r1                    //  [ B3 ]  [ B2 B1 B0 ...   A6  0 B6 C6 D6 ]
/* 26 +  1 */  adcs r4, r4                    //  [ E6 ]  [ E5 E4 E3 E2 E1 E0 ...   A3 B3 ]
/* 27 +  1 */  adcs r1, r1                    //  [ B2 ]  [ B1 B0 ...   A6  0 B6 C6 D6 E6 ]
/* 28 +  1 */  mov r7, r8
/* 29 +  1 */  adcs r7, r7                    //  [ F6 ]  [ F5 F4 F3 F2 F1 F0 ...   A2 B2 ]
/* 30 +  1 */  mov r8, r7
/* 31 +  1 */  adcs r1, r1                    //  [ B1 ]  [ B0 ...   A6  0 B6 C6 D6 E6 F6 ]
/* 32 +  1 */  mov r7, r9
/* 33 +  1 */  adcs r7, r7                    //  [ G6 ]  [ G5 G4 G3 G2 G1 G0 ...   A1 B1 ]
/* 34 +  1 */  mov r9, r7
/* 35 +  1 */  adcs r1, r1                    //  [ B0 ]  [ ...   A6  0 B6 C6 D6 E6 F6 G6 ]
/* 36 +  1 */  adcs r5, r5                    //  [ H6 ]  [ H5 H4 H3 H2 H1 H0 ...   A0 B0 ]
/* 37 +  1 */  adcs r1, r1                    //  [  0 ]  [ ...A6  0 B6 C6 D6 E6 F6 G6 H6 ]

/* 38 +  1 */  uxtb r0, r1                    //          [ ...      B6 C6 D6 E6 F6 G6 H6 ]
/* 39 +  1 */  adds r1, r0                    //  [  0 ]  [ ...A6 B6 C6 D6 E6 F6 G6 H6  0 ]
/* 40 +  1 */  lsrs r1, r1, #1                //  [  0 ]  [ ...   A6 B6 C6 D6 E6 F6 G6 H6 ]
/* 41 +  1 */  movs r0, #0

/* r1: [A-H]6 (done) */

/* phase C (T1L) */
                                             //  carry   [           register            ]
/* 42 +  2 */  strh r0, [r6, #0] // 0x00

/* 44 +  1 */  movs r0, #255
/* 45 +  1 */  adds r2, r2                    //  [ C5 ]  [ C4 C3 C2 C1 C0 ...   A5 B5  0 ]
/* 46 +  1 */  adcs r2, r2                    //  [ C4 ]  [ C3 C2 C1 C0 ...   A5 B5  0 C5 ]
/* 47 +  1 */  adcs r3, r3                    //  [ D5 ]  [ D4 D3 D2 D1 D0 ...   A4 B4 C4 ]
/* 48 +  1 */  adcs r2, r2                    //  [ C3 ]  [ C2 C1 C0 ...   A5 B5  0 C5 D5 ]
/* 49 +  1 */  adcs r4, r4                    //  [ E5 ]  [ E4 E3 E2 E1 E0 ...   A3 B3 C3 ]
/* 50 +  1 */  adcs r2, r2                    //  [ C2 ]  [ C1 C0 ...   A5 B5  0 C5 D5 E5 ]
/* 51 +  1 */  mov r7, r8
/* 52 +  1 */  adcs r7, r7                    //  [ F5 ]  [ F4 F3 F2 F1 F0 ...   A2 B2 C2 ]
/* 53 +  1 */  mov r8, r7
/* 54 +  1 */  adcs r2, r2                    //  [ C1 ]  [ C0 ...   A5 B5  0 C5 D5 E5 F5 ]
/* 55 +  1 */  mov r7, r9
/* 56 +  1 */  adcs r7, r7                    //  [ G5 ]  [ G4 G3 G2 G1 G0 ...   A1 B1 C1 ]
/* 57 +  1 */  adcs r2, r2                    //  [ C0 ]  [ ...   A5 B5  0 C5 D5 E5 F5 G5 ]
/* 58 +  1 */  adcs r5, r5                    //  [ H5 ]  [ H4 H3 H2 H1 H0 ...   A0 B0 C0 ]
/* 59 +  1 */  adcs r2, r2                    //  [  0 ]  [ ...A5 B5  0 C5 D5 E5 F5 G5 H5 ]

bitbang_bit_6:
/*  0 +  2 */  strh r0, [r6, #0] // 0xff

/*  2 +  1 */  lsls r2, r2, #1                //  [  0 ]  [ ...A5 B5  0 C5 D5 E5 F5 G5 H5 0 ]
/*  3 +  1 */  uxtb r0, r2                    //          [ ...         C5 D5 E5 F5 G5 H5 0 ]
/*  4 +  1 */  adds r2, r0                    //  [  0 ]  [ ...A5 B5 C5 D5 E5 F5 G5 H5  0 0 ]
/*  5 +  1 */  lsrs r2, r2, #2                //  [  0 ]  [ ...   A5 B5 C5 D5 E5 F5 G5 H5 ]

/*  6 +  1 */  mov r0, r8

/* r0: F
 * r1: [A-H]6 (done)
 * r2: [A-H]5 (done)
 * r3: D
 * r4: E
 * r5: H
 * r6: &GPIO_OUT
 * r7: G
 */

/*  7 +  1 */  adds r3, r3                    //  [ D4 ]  [ D3 D2 D1 D0 ...   A4 B4 C4  0 ]
/*  8 +  1 */  adcs r3, r3                    //  [ D3 ]  [ D2 D1 D0 ...   A4 B4 C4  0 D4 ]
/*  9 +  1 */  adcs r4, r4                    //  [ E4 ]  [ E3 E2 E1 E0 ...   A3 B3 C3 D3 ]
/* 10 +  1 */  adcs r3, r3                    //  [ D2 ]  [ D1 D0 ...   A4 B4 C4  0 D4 E4 ]
/* 11 +  1 */  adcs r0, r0                    //  [ F4 ]  [ F3 F2 F1 F0 ...   A2 B2 C2 D2 ]
/* 12 +  1 */  adcs r3, r3                    //  [ D1 ]  [ D0 ...   A4 B4 C4  0 D4 E4 F4 ]
/* 13 +  1 */  adcs r7, r7                    //  [ G4 ]  [ G3 G2 G1 G0 ...   A1 B1 C1 D1 ]
/* 14 +  1 */  adcs r3, r3                    //  [ D0 ]  [ ...   A4 B4 C4  0 D4 E4 F4 G4 ]
/* 15 +  1 */  adcs r5, r5                    //  [ H4 ]  [ H3 H2 H1 H0 ...   A0 B0 C0 D0 ]
/* 16 +  1 */  adcs r3, r3                    //  [  0 ]  [ ...A4 B4 C4  0 D4 E4 F4 G4 H4 ]
/* 17 +  1 */  lsls r3, r3, #2                //  [  0 ]  [ ...A4 B4 C4  0 D4 E4 F4 G4 H4 0 0 ]

/* 18 +  2 */  strh r1, [r6, #0]

/* r1: free */

/* 20 +  1 */  uxtb r1, r3                    //          [ ...            D4 E4 F4 G4 H4 0 0 ]
/* 21 +  1 */  adds r3, r1                    //  [  0 ]  [ ...A4 B4 C4 D4 E4 F4 G4 H4  0 0 0 ]
/* 22 +  1 */  lsrs r3, r3, #3                //  [  0 ]  [ ...       A4 B4 C4 D4 E4 F4 G4 H4 ]

/* r3: [A-H]4 (done) */

/* 23 +  1 */  adds r4, r4                    //  [ E3 ]  [ E2 E1 E0 ...   A3 B3 C3 D3  0 ]
/* 24 +  1 */  adcs r4, r4                    //  [ E2 ]  [ E1 E0 ...   A3 B3 C3 D3  0 E3 ]
/* 25 +  1 */  adcs r0, r0                    //  [ F3 ]  [ F2 F1 F0 ...   A2 B2 C2 D2 E2 ]
/* 26 +  1 */  adcs r4, r4                    //  [ E1 ]  [ E0 ...   A3 B3 C3 D3  0 E3 F3 ]
/* 27 +  1 */  adcs r7, r7                    //  [ G3 ]  [ G2 G1 G0 ...   A1 B1 C1 D1 E1 ]
/* 28 +  1 */  adcs r4, r4                    //  [ E0 ]  [ ...   A3 B3 C3 D3  0 E3 F3 G3 ]
/* 29 +  1 */  adcs r5, r5                    //  [ H3 ]  [ H2 H1 H0 ...   A0 B0 C0 D0 E0 ]
/* 30 +  1 */  adcs r4, r4                    //  [  0 ]  [ ...A3 B3 C3 D3  0 E3 F3 G3 H3 ]

/* 31 +  1 */  lsls r4, r4, #3                //  [  0 ]  [ ...A3 B3 C3 D3  0 E3 F3 G3 H3 0 0 0 ]
/* 32 +  1 */  uxtb r1, r4                    //          [ ...               E3 F3 G3 H3 0 0 0 ]
/* 33 +  1 */  adds r4, r1                    //  [  0 ]  [ ...A3 B3 C3 D3 E3 F3 G3 H3  0 0 0 0 ]
/* 34 +  1 */  lsrs r4, r4, #4                //  [  0 ]  [ ...         A3 B3 C3 D3 E3 F3 G3 H3 ]

/* 35 +  1 */  movs r1, #0
/* r4: [A-H]3 (done) */

/* 36 +  1 */  adds r0, r0                    //  [ F2 ]  [ F1 F0 ...   A2 B2 C2 D2 E2  0 ]
/* 37 +  1 */  adcs r0, r0                    //  [ F1 ]  [ F0 ...   A2 B2 C2 D2 E2  0 F2 ]
/* 38 +  1 */  adcs r7, r7                    //  [ G2 ]  [ G1 G0 ...   A1 B1 C1 D1 E1 F1 ]
/* 39 +  1 */  adcs r0, r0                    //  [ F0 ]  [ ...   A2 B2 C2 D2 E2  0 F2 G2 ]
/* 40 +  1 */  adcs r5, r5                    //  [ H2 ]  [ H1 H0 ...   A0 B0 C0 D0 E0 F0 ]
/* 41 +  1 */  adcs r0, r0                    //  [  0 ]  [ ...A2 B2 C2 D2 E2  0 F2 G2 H2 ]

/* phase C (T1L) */
                                              //  carry   [           register            ]
/* 42 +  2 */  strh r1, [r6, #0]  // 0x00
/* 44 +  1 */  lsls r0, r0, #4                //  [  0 ]  [ ...A2 B2 C2 D2 E2  0 F2 G2 H2 0 0 0 0 ]
/* 45 +  1 */  uxtb r1, r0                    //          [ ...                  F2 G2 H2 0 0 0 0 ]
/* 46 +  1 */  adds r0, r1                    //  [  0 ]  [ ...A2 B2 C2 D2 E2 F2 G2 H2  0 0 0 0 0 ]
/* 47 +  1 */  lsrs r0, r0, #5                //  [  0 ]  [ ...           A2 B2 C2 D2 E2 F2 G2 H2 ]

/* r0: [A-H]2 (done)
 * r1: free
 * r2: [A-H]5 (done)
 * r3: [A-H]4 (done)
 * r4: [A-H]3 (done)
 * r5: H
 * r6: &GPIO_OUT
 * r7: G
 */

/* 48 +  1 */  adds r7, r7                    //  [ G1 ]  [ G0 ...   A1 B1 C1 D1 E1 F1  0 ]
/* 49 +  1 */  adcs r7, r7                    //  [ G0 ]  [ ...   A1 B1 C1 D1 E1 F1  0 G1 ]
/* 50 +  1 */  adcs r5, r5                    //  [ H1 ]  [ H0 ...   A0 B0 C0 D0 E0 F0 G0 ]
/* 51 +  1 */  adcs r7, r7                    //  [  0 ]  [ ...A1 B1 C1 D1 E1 F1  0 G1 H1 ]
/* 52 +  1 */  lsls r7, r7, #5                //  [  0 ]  [ ...A1 B1 C1 D1 E1 F1  0 G1 H1 0 0 0 0 0 ]
/* 53 +  1 */  uxtb r1, r7                    //          [ ...                     G1 H1 0 0 0 0 0 ]
/* 54 +  1 */  adds r7, r1                    //  [  0 ]  [ ...A1 B1 C1 D1 E1 F1 G1 H1  0 0 0 0 0 0 ]
/* 55 +  1 */  lsrs r7, r7, #6                //  [  0 ]  [ ...             A1 B1 C1 D1 E1 F1 G1 H1 ]
/* 56 +  1 */  lsls r1, r5, #1                //  [ H0 ]
/* 57 +  1 */  adcs r5, r5                    //  [ H0 ]  [ ...   A0 B0 C0 D0 E0 F0 G0 H0 ]
/* 58 +  1 */  movs r1, #255

/* 59 +  1 */  mov r9, r7

/* r0: [A-H]2 (done)
 * r1: 0xff
 * r2: [A-H]5 (done)
 * r3: [A-H]4 (done)
 * r4: [A-H]3 (done)
 * r5: [A-H]0 (done)
 * r6: &GPIO_OUT
 * r7:
 * r8:
 * r9: [A-H]1 (done)
 * r10: &buffer[current_byte_of_first_strip]
 * r11: current_byte_of_first_strip
 */

bitbang_bit_5:
/*  0 +  2 */  strh r1, [r6, #0] // 0xff
/*  2 + 16 */  delay r7, 16
/* 18 +  2 */  strh r2, [r6, #0] // [A-H]5 (done)
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 16 */  delay r7, 16
bitbang_bit_4:
/*  0 +  2 */  strh r1, [r6, #0] // 0xff
/*  2 + 16 */  delay r7, 16
/* 18 +  2 */  strh r3, [r6, #0] // [A-H]4 (done)
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 16 */  delay r7, 16
bitbang_bit_3:
/*  0 +  2 */  strh r1, [r6, #0] // 0xff
/*  2 + 16 */  delay r7, 16
/* 18 +  2 */  strh r4, [r6, #0] // [A-H]3 (done)
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 16 */  delay r7, 16
bitbang_bit_2:
/*  0 +  2 */  strh r1, [r6, #0] // 0xff
/*  2 +  3 */  delay r7, 3
/*  5 +  1 */  movs r7, #N_VALUES_PER_STRIP
/*  6 +  1 */  cmp  r7, r11
/*  7 +  1 | 3 */ beq 1f
/*  8 +  3 */  b bitbang_bit_2_cycle_14
1:
/* 10 +  8 */  delay r7, 8
/* 18 +  2 */  strh r0, [r6, #0] // [A-H]2 (done)
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 16 */  delay r7, 16
bitbang_bit_1_epilogue:
/*  0 +  2 */  strh r1, [r6, #0] // 0xff
/*  2 +  1 */  mov r0, r9
/*  3 + 15 */  delay r7, 15
/* 18 +  2 */  strh r0, [r6, #0] // [A-H]1 (done)
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 16 */  delay r7, 16
bitbang_bit_0_epilogue:
/*  0 +  2 */  strh r1, [r6, #0] // 0xff
/*  2 + 16 */  delay r7, 16
/* 18 +  2 */  strh r5, [r6, #0] // [A-H]0 (done)
/* 20 + 22 */  delay r7, 22
/* 42 +  2 */  strh r7, [r6, #0] // 0
/* 44 + 16 */  delay r7, 16


bitbang_end:

/* pop r4-r11 */

pop {r4-r7}
mov r8, r4
mov r9, r5
mov r10, r6
mov r11, r7
pop {r4-r7}

bx lr

	.size	bitbang, .-bitbang
