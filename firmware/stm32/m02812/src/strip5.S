
#include "fsm.h"
#include "util.h"
#include "take5_init.h"
#include "bitbang16_5strip.h"

#if N_STRIPS == 5
#error bad number of strips
#endif

#define PERIPH_BASE         (                      0x40000000 )
#define AHBPERIPH_BASE      ( PERIPH_BASE        + 0x00020000 )
#define DMA1_BASE           ( AHBPERIPH_BASE     + 0x00000000 )
#define DMA1_CHANNEL3_BASE  ( DMA1_BASE          + 0x00000030 )
#define DMA1_CHANNEL3_CNDTR ( DMA1_CHANNEL3_BASE + 0x00000004 )

.include "m0delay.S"

    .cpu cortex-m0
    .fpu softvfp
    .section    .bss
    /* create symbols for automated tests */
    .set VALUE_COUNT, N_VALUES
    .set SEGMENT_COUNT, N_SEGMENTS
    .set STRIP_COUNT, N_STRIPS
    .thumb
    .section    .text.bitbang,"ax",%progbits
    .balign 2
    .global main
    .code   16
    .thumb_func
    .type   main, %function
    .syntax unified
    .thumb

main:

bl init

post_init:

#define rGPIO_OUT  r6
#define rWRITE_BUF r8
#define rREAD_BUF  r9
#define rWRITE_IX  r10
#define rREAD_IX   r11

ldr rGPIO_OUT, =GPIOA_ODR
mov r0, #0
mov rREAD_IX, r0
ldr r0, =frame_a
mov rWRITE_BUF, r0
ldr r0, =frame_b
mov rREAD_BUF, r0

main_loop:

bitbang_start:

mov r2, #0
mov r3, #0
mov rWRITE_IX, r2

bitbang_bit_1_cycle54:
//
//   r0:  ?
//   r1:  ?
//   r2:  lo[ A0 B0 C0 D0 E0 ] || 0
//   r3:  lo[  1  1  1  1  1 ] || 0
//   r4:  ?
//   r5:  ?
//   r6:  rGPIO_OUT
//   r7:  ?
//   r8:  rWRITE_BUF
//   r9:  rREAD_BUF
//   r10: rWRITE_IX
//   r11: rREAD_IX
//

/* 54 +  2 */  ldr r5, =remainders

bitbang_bit_0: /* last bit of the last byte */

/*  0 +  2 */  strh r3, [rGPIO_OUT] // [ 1 1 1 1 1 ] || [ 0 0 0 0 0 ] (first iteration)

/*  2 +  1 */  add r5, rWRITE_IX
/*  3 +  1 */  mov r7, rWRITE_BUF
/*  4 +  1 */  add r7, rWRITE_IX
/*  5 +  1 */  add r7, rWRITE_IX
/*  6 +  2 */  ldrh r0, [r7, #0]
/*  8 +  2 */  ldrb r4, [r5, #0]
/* 10 +  1 */  adds r0, r4
/* 11 +  2 */  strb r0, [r5, #0]

/* 13 +  2 */  strh r2, [rGPIO_OUT] // data

/* 15 +  1 */  lsrs r0, r0, #8                 // r0: [ 0 ... A[7-0] ]
/* 16 +  1 */  mov r3, #0

/* 17 +  2 */  ldrh r1, [r7, #2]
/* 19 +  2 */  ldrb r4, [r5, #1]
/* 21 +  1 */  adds r1, r4
/* 22 +  2 */  strb r1, [r5, #1]

/* 24 +  2 */  ldrh r2, [r7, #4]
/* 26 +  2 */  ldrb r4, [r5, #2]

/* 28 +  2 */  strh r3, [rGPIO_OUT] // 0

/* 30 +  1 */  lsrs r1, r1, #8                 // r1: [ 0 ... B[7-0] ]
/* 31 +  1 */  adds r2, r4
/* 32 +  2 */  strb r2, [r5, #2]
/* 34 +  1 */  lsrs r2, r2, #8                 // r2: [ 0 ... C[7-0] ]

/* 35 +  2 */  ldrh r3, [r7, #6]
/* 37 +  2 */  ldrb r4, [r5, #3]
/* 39 +  1 */  adds r3, r4
/* 40 +  2 */  strb r3, [r5, #3]
/* 42 +  1 */  lsrs r3, r3, #8                 // r3: [ 0 ... D[7-0] ]

/* 43 +  2 */  ldrh r4, [r7, #8]
/* 45 +  2 */  ldrb r7, [r5, #4]
/* 47 +  1 */  adds r4, r7
/* 48 +  2 */  strb r4, [r5, #4]
/* 50 +  1 */  lsrs r4, r4, #8                 // r4: [ 0 ... E[7-0] ]

//
//   r0:  [ 0... A6 A5 A4 A3 A2 A1 A0 ]
//   r1:  [ 0... B6 B5 B4 B3 B2 B1 B0 ]
//   r2:  [ 0... C6 C5 C4 C3 C2 C1 C0 ]
//   r3:  [ 0... D6 D5 D4 D3 D2 D1 D0 ]
//   r4:  [ 0... E6 E5 E4 E3 E2 E1 E0 ]
//   r5:  ?
//   r6:  rGPIO_OUT
//   r7:  ?
//   r8:  rWRITE_BUF
//   r9:  rREAD_BUF
//   r10: rWRITE_IX
//   r11: rREAD_IX
//

/* 51 +  1 */  lsls r4, r4, #24               //          [ E7 E6 E5 E4 E3 E2 E1 E0 ... 0 ]
/* 52 +  1 */  lsls r3, r3, #24               //          [ D7 D6 D5 D4 D3 D2 D1 D0 ... 0 ]
/* 53 +  1 */  lsls r2, r2, #24               //          [ C7 C6 C5 C4 C3 C2 C1 C0 ... 0 ]
/* 54 +  1 */  lsls r1, r1, #24               //          [ B7 B6 B5 B4 B3 B2 B1 B0 ... 0 ]
/* 55 +  1 */  mov r5, #31                    //          [ 0          ...  1  1  1  1  1 ]

bitbang_bit_7:

/*  0 +  2 */  strh r5, [rGPIO_OUT] // 0x1f
/*  2 +  1 */  lsls r0, r0, #25               //  [ A7 ]  [ A6 A5 A4 A3 A2 A1 A0    ... 0 ]
/*  3 +  1 */  adcs r0, r0                    //  [ A6 ]  [ A5 A4 A3 A2 A1 A0      ... A7 ]
/*  4 +  1 */  adcs r1, r1                    //  [ B7 ]  [ B6 B5 B4 B3 B2 B1 B0 ...   A6 ]
/*  5 +  1 */  adcs r0, r0                    //  [ A5 ]  [ A4 A3 A2 A1 A0      ... A7 B7 ]
/*  6 +  1 */  adcs r2, r2                    //  [ C7 ]  [ C6 C5 C4 C3 C2 C1 C0 ...   A5 ]
/*  7 +  1 */  adcs r0, r0                    //  [ A4 ]  [ A3 A2 A1 A0      ... A7 B7 C7 ]
/*  8 +  1 */  adcs r3, r3                    //  [ D7 ]  [ D6 D5 D4 D3 D2 D1 D0 ...   A4 ]
/*  9 +  1 */  adcs r0, r0                    //  [ A3 ]  [ A2 A1 A0      ... A7 B7 C7 D7 ]
/* 10 +  1 */  adcs r4, r4                    //  [ E7 ]  [ E6 E5 E4 E3 E2 E1 E0 ...   A3 ]
/* 11 +  1 */  adcs r0, r0                    //  [ A2 ]  [ A1 A0      ... A7 B7 C7 D7 E7 ]
/* 12 +  1 */  mov r7, #0
/* 13 +  2 */  strh r0, [rGPIO_OUT] // most significant data bits
/* 15 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0                      ... A2 ]
/* 16 +  1 */  bics r0, r5                    //  [  0 ]  [ A1 A0                  ...  0 ]
/* 17 +  1 */  adcs r1, r1                    //  [ B6 ]  [ B5 B4 B3 B2 B1 B0 ...   A6  0 ]
/* 18 +  1 */  adcs r1, r1                    //  [ B5 ]  [ B4 B3 B2 B1 B0 ...   A6  0 B6 ]
/* 19 +  1 */  adcs r2, r2                    //  [ C6 ]  [ C5 C4 C3 C2 C1 C0 ...   A5 B5 ]
/* 20 +  1 */  adcs r1, r1                    //  [ B4 ]  [ B3 B2 B1 B0 ...   A6  0 B6 C6 ]
/* 21 +  1 */  adcs r3, r3                    //  [ D6 ]  [ D5 D4 D3 D2 D1 D0 ...   A4 B4 ]
/* 22 +  1 */  adcs r1, r1                    //  [ B3 ]  [ B2 B1 B0 ...   A6  0 B6 C6 D6 ]
/* 23 +  1 */  adcs r4, r4                    //  [ E6 ]  [ E5 E4 E3 E2 E1 E0 ...   A3 B3 ]
/* 24 +  1 */  adcs r1, r1                    //  [ B2 ]  [ B1 B0 ...   A6  0 B6 C6 D6 E6 ]
/* 25 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0                   ... A2 B2 ]
/* 26 +  1 */  ands r5, r1                    //  [  0 ]  [ 0             ... B6 C6 D6 E6 ]
/* 27 +  1 */  add  r1, r5                    //          [ B1 B0 ...   A6 B6 C6 D6 E6  0 ]
/* 28 +  2 */  strh r0, [rGPIO_OUT] // 0
/* 30 +  1 */  lsrs r1, #1                    //  [  0 ]  [  0 B1 B0 ...   A6 B6 C6 D6 E6 ]
/* 31 +  1 */  adcs r2, r2                    //  [ C5 ]  [ C4 C3 C2 C1 C0 ...   A5 B5  0 ]
/* 32 +  1 */  adcs r2, r2                    //  [ C4 ]  [ C3 C2 C1 C0 ...   A5 B5  0 C5 ]
/* 33 +  1 */  adcs r3, r3                    //  [ D5 ]  [ D4 D3 D2 D1 D0 ...   A4 B4 C4 ]
/* 34 +  1 */  adcs r2, r2                    //  [ C3 ]  [ C2 C1 C0 ...   A5 B5  0 C5 D5 ]
/* 35 +  1 */  adcs r4, r4                    //  [ E5 ]  [ E4 E3 E2 E1 E0 ...   A3 B3 C3 ]
/* 36 +  1 */  adcs r2, r2                    //  [ C2 ]  [ C1 C0 ...   A5 B5  0 C5 D5 E5 ]
/* 37 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0                ... A2 B2 C2 ]
/* 38 +  1 */  adcs r3, r3                    //  [ D4 ]  [ D3 D2 D1 D0 ...   A4 B4 C4  0 ]
/* 39 +  1 */  adcs r3, r3                    //  [ D3 ]  [ D2 D1 D0 ...   A4 B4 C4  0 D4 ]
/* 40 +  1 */  adcs r4, r4                    //  [ E4 ]  [ E3 E2 E1 E0 ...   A3 B3 C3 D3 ]
/* 41 +  1 */  adcs r3, r3                    //  [ D2 ]  [ D1 D0 ...   A4 B4 C4  0 D4 E4 ]
/* 42 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0             ... A2 B2 C2 D2 ]
/* 43 +  1 */  mov r5, #31                    //          [ 0          ...  1  1  1  1  1 ]
/* 44 +  1 */  rors r4, r4, r5                //  [ E3 ]  [ E2 E1 E0 ...   A3 B3 C3 D3 E3 ] r5 val coincidence!
/* 45 +  1 */  rors r0, r0, r5                //  [ A1 ]  [ A0                         A1 ] r5 val coincidence!
/* 46 +  1 */  lsls r4, r4                    //  [ E2 ]  [ E1 E0 ...   A3 B3 C3 D3 E3  0 ]
/* 47 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0          ... A2 B2 C2 D2 E2 ]
/* 48 +  1 */  lsls r7, #11                   //  [  0 ]  [ 0          ... A2 B2 C2 D2 E2  ... [0]*11 ]
/* 49 +  1 */  lsls r1, #2                    //  [ B1 ]  [ B0 ...   A6 B6 C6 D6 E6  0  0 ]
/* 50 +  1 */  adcs r0, r0                    //  [ A0 ]  [ 0 ...                   A1 B1 ]
/* 51 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0          ... A2 B2 C2 D2 E2 ... [0]*11 ... A0 ]
/* 52 +  1 */  adcs r2, r2                    //  [ C1 ]  [ C0 ...   A5 B5  0 C5 D5 E5  0 ]
/* 53 +  1 */  adcs r0, r0                    //  [  0 ]  [ 0 ...                A1 B1 C1 ]
/* 54 +  1 */  adcs r3, r3                    //  [ D1 ]  [ D0 ...   A4 B4 C4  0 D4 E4  0 ]
/* 55 +  1 */  adcs r0, r0                    //  [  0 ]  [ 0 ...             A1 B1 C1 D1 ]

bitbang_bit_6:

/*  0 +  2 */  strh r5, [rGPIO_OUT] // 0x1f
/*  2 +  1 */  bics r5, r2                    //  [  0 ]  [ 0 ...             C5 D5 E5  0 ]
/*  3 +  1 */  adcs r2, r5                    //  [  0 ]  [ C0 ...   A5 B5 C5 D5 E5  0  0 ]
/*  4 +  1 */  adcs r4, r4                    //  [ E1 ]  [ E0 ...   A3 B3 C3 D3 E3  0  0 ]
/*  5 +  1 */  adcs r0, r0                    //  [  0 ]  [ 0 ...          A1 B1 C1 D1 E1 ]
/*  6 +  1 */  mov r5, #31                    //  [  0 ]  [ 0          ...  1  1  1  1  1 ]
/*  7 +  1 */  lsls r1, #1                    //  [ B0 ]  [ 0 ... A6 B6 C6 D6 E6  0  0  0 ]
/*  8 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0 ... A2 B2 C2 D2 E2 ... [0]*11 ... A0 B0 ]
/*  9 +  1 */  lsrs r1, #3                    //  [  0 ]  [ 0 ...          A6 B6 C6 D6 E6 ]
/* 10 +  1 */  adcs r2, r5                    //  [ C0 ]  [ 0 ... A5 B5 C5 D5 E5  0  0  0 ]
/* 11 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0 ... A2 B2 C2 D2 E2 ... [0]*11 ... A0 B0 C0 ]
/* 12 +  1 */  adcs r3, r3                    //  [ D0 ]  [ 0 ... A4 B4 C4  0 D4 E4  0  0 ]
/* 13 +  2 */  strh r1, [rGPIO_OUT] // bits 6
/* 15 +  1 */  adcs r7, r7                    //  [  0 ]  [ 0 ... A2 B2 C2 D2 E2 ... [0]*11 ... A0 B0 C0 D0 ]
/* 16 +  1 */  adcs r4, r4                    //  [ E0 ]  [ 0 ... A3 B3 C3 D3 E3  0  0  0 ]
/* 17 +  1 */  adcs r7, r7                    //  [  0 ]  hi[ A2 B2 C2 D2 E2 ] lo[ A0 B0 C0 D0 E0 ]
/* 18 +  1 */  bics r5, r3                    //  [  0 ]  [ 0 ...           0 D4 E4  0  0 ]
/* 19 +  1 */  adcs r3, r5                    //  [  0 ]  [ 0 ... A4 B4 C4 D4 E4  0  0  0 ]
/* 20 +  1 */  lsrs r2, #3                    //  [  0 ]  [ 0 ...          A5 B5 C5 D5 E5 ]
/* 21 +  1 */  lsls r3, #13                   //  [  0 ]  hi[ A4 B4 C4 D4 E4 ] lo[0]
/* 22 +  1 */  lsls r4, #13                   //  [  0 ]  hi[ A3 B3 C3 D3 E3 ] lo[0]
/* 23 +  1 */  add r2, r3                     //  [  0 ]  hi[ A4 B4 C4 D4 E4 ] lo[ A5 B5 C5 D5 E5 ]
/* 24 +  1 */  add r0, r4                     //  [  0 ]  hi[ A3 B3 C3 D3 E3 ] lo[ A1 B1 C1 D1 E1 ]
//
//   r0:  hi[ A3 B3 C3 D3 E3 ] lo[ A1 B1 C1 D1 E1 ]
//   r1:                       lo[  0  0  0  0  0 ]
//   r2:  hi[ A4 B4 C4 D4 E4 ] lo[ A5 B5 C5 D5 E5 ]
//   r3:                       lo[  1  1  1  1  1 ]
//   r4:  ?
//   r5:  ?
//   r6:  rGPIO_OUT
//   r7:  hi[ A2 B2 C2 D2 E2 ] lo[ A0 B0 C0 D0 E0 ]
//   r8:  rWRITE_BUF
//   r9:  rREAD_BUF
//   r10: rWRITE_IX
//   r11: rREAD_IX
//  

#define rTMP r3  /*  */

/* 25 +  1 */  nop
/* 26 +  1 */  nop
/* 27 +  1 */                                          mov rTMP, #0
/* 28 +  2 */                                          strh rTMP, [rGPIO_OUT]


              /* free: r1, r4, r5 */
/* 30 + 25 */  delay rTMP, 25

/* 55 +  1 */                                          mov rTMP, #31
bitbang_bit_5:
/*  0 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/*  2 + 10 */  delay rTMP, 10

/* 12 +  1 */                                          nop
/* 13 +  2 */                                          strh r2, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 15 + 12 */  delay rTMP, 12

/* 27 +  1 */                                          mov rTMP, #0
/* 28 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 30 + 25 */  delay rTMP, 25

/* 55 +  1 */                                          mov rTMP, #31
bitbang_bit_4:
/*  0 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/*  2 + 10 */  delay rTMP, 10

/* 12 +  1 */                                          lslr rTMP, r2, #16
/* 13 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 15 + 12 */  delay rTMP, 12

/* 27 +  1 */                                          mov rTMP, #0
/* 28 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 30 + 25 */  delay rTMP, 25

/* 55 +  1 */                                          mov rTMP, #31

bitbang_bit_3:
/*  0 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/*  2 + 10 */  delay rTMP, 10

/* 12 +  1 */                                          lslr rTMP, r0, #16
/* 13 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 15 + 12 */  delay rTMP, 12

/* 27 +  1 */                                          mov rTMP, #0
/* 28 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 30 + 25 */  delay rTMP, 25

/* 55 +  1 */                                          mov rTMP, #31

bitbang_bit_2:
/*  0 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/*  2 + 10 */  delay rTMP, 10

/* 12 +  1 */                                          lslr rTMP, r7, #16
/* 13 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 15 + 12 */  delay rTMP, 12

/* 27 +  1 */                                          mov rTMP, #0
/* 28 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 30 + 25 */  delay rTMP, 25

/* 55 +  1 */                                          mov rTMP, #31

bitbang_bit_1:
/*  0 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/*  2 + 10 */  delay rTMP, 10

/* 13 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 15 + 12 */  delay rTMP, 12

/* 27 +  1 */                                          mov rTMP, #0
/* 28 +  2 */                                          strh rTMP, [rGPIO_OUT]

              /* free: r1, r4, r5 */
/* 30 + 25 */  delay rTMP, 25

/* 55 +  1 */                                          mov rTMP, #31







// conditional

#unset rTMP

/* 45 +  1 */                                          utxb r2, r7
/* 46 +  1 */                                          mov r3, #31
/* 47 +  1 | 3 */ beq 1f
/* 48 +  6 */  b bitbang_bit_1_cycle54
1:
/* 50 +  6 */  delay r1, 5
bitbang_bit_0_epiloque:
/*  0 +  2 */                                          strh r3, [rGPIO_OUT]
/*  2 + 11 */  delay rTMP, 11
/* 13 +  2 */                                          strh r2, [rGPIO_OUT]
/* 15 + 12 */  delay rTMP, 12
/* 27 +  1 */                                          mov r3, #0
/* 28 +  2 */                                          strh rTMP, [rGPIO_OUT]
bitbang_end:




