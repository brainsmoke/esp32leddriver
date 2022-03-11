
#include <jitter_timer.h>

.include "m0delay.S"
	.cpu cortex-m0
	.fpu softvfp
	.section	.bss
	.comm histogram,(HISTOGRAM_MAX+1)*4
	.set FRAME_LENGTH, FRAME_LEN
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.balign	4
	.code	16

	.global	bitbang
	.code	16
	.thumb_func
	.type	bitbang, %function
	.syntax unified
	.thumb


bitbang:
/* push r4-r11 */

push {r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
push {r4-r7}

#define rTMP        r0
#define rSCRATCH    r1
#define rGPIO       r2
#define   GPIO_ODR  (0x14)

#define rTIMER      r3
#define   TIM_SR    (0x10)
#define   TIM_CCR1  (0x34)


#define   TIM_SR_UIF   (1)
#define   TIM_SR_CC1IF (2)

#define rLAST    r4
#define rOUTDATA r5
#define rPINMASK r6
#define rOUTTMP  r7

#define rHISTOGRAM r8
#define rPIN       r9
#define rBUFFER    r10
#define rBUFEND    r11

bitbang_start:

mov rBUFFER, r0

mov rPIN, r1
movs rPINMASK, #1
lsls rPINMASK, r1

ldr rTMP, =histogram
mov rHISTOGRAM, rTMP

ldr rLAST, [rTIMER, #TIM_CCR1]

ldr rTMP, =(FRAME_LEN*2)
add rTMP, rBUFFER
mov rBUFEND, rTMP

b bitbang_load

.macro capture_check_start_14_cycles

/*  0 +  4!*/  ldr rTMP, [rTIMER, #TIM_CCR1]
/*  4 +  1 */  subs rTMP, rLAST
/*  5 +  1 */  adds rLAST, rTMP
/*  6 +  1 */  uxth rTMP, rTMP
/*  7 +  1 */  cmp rTMP, #HISTOGRAM_MAX
/*  8 +  1 */  sbcs rSCRATCH, rSCRATCH
/*  9 +  1 */  ands rTMP, rSCRATCH
/* 10 +  1 */  lsls rTMP, rTMP, #2
/* 11 +  1 */  add rTMP, rHISTOGRAM
/* 12 +  2 */  ldr rSCRATCH, [rTMP]
.endm

.macro capture_check_end_3_cycles
/*  0 +  1 */  adds rSCRATCH, #1
/*  1 +  2 */  str rSCRATCH, [rTMP]
.endm

.macro capture_check_17_cycles
/*  0 + 14 */  capture_check_start_14_cycles
/* 14 +  3 */  capture_check_end_3_cycles
.endm



.macro bitbang_pulse_start_2_cycles
/*  0 +  2 */  strh rPINMASK, [rGPIO, #GPIO_ODR]
.endm

.macro bitbang_pulse_end_59_cycles bit
/*  0 + 14 */  capture_check_start_14_cycles
/* 14 +  1 */  lsrs rOUTTMP, rOUTDATA, #(\bit)
/* 15 +  1 */  ands rOUTTMP, rPINMASK
/* 16 +  2 */  strh rOUTTMP, [rGPIO, #GPIO_ODR]

/* 18 +  3 */  capture_check_end_3_cycles
/* 21 +  1 */  nop
/* 24 + 15 */  capture_check_17_cycles

/* 39 +  1 */  movs rOUTTMP, #0
/* 40 +  2 */  strh rOUTTMP, [rGPIO, #GPIO_ODR]
/* 42 + 17 */  capture_check_17_cycles
.endm

.macro bitbang_pulse_61_cycles bit
/*  0 +  2 */  bitbang_pulse_start_2_cycles
/*  2 + 59 */  bitbang_pulse_end_59_cycles \bit
.endm


.macro bitbang_pulse_70_cycles bit
/*  0 + 61 */  bitbang_pulse_61_cycles \bit
/* 61 +  9 */  delay rOUTTMP, 9
.endm


bitbang_epilogue:
capture_check_17_cycles
capture_check_17_cycles
b bitbang_end

bitbang_0:
/*  0 + 61 */  bitbang_pulse_61_cycles 0
/* 61 +  1 */  cmp rBUFFER, rBUFEND
/* 62 + 1|6*/  beq bitbang_epilogue
bitbang_load:
bitbang_0_cycle_63:
/* 63 +  1 */  mov rOUTTMP, rBUFFER
/* 64 +  2 */  ldrh rOUTDATA, [rOUTTMP]
/* 66 +  1 */  mov rOUTTMP, rPIN
/* 67 +  1 */  lsls rOUTDATA, rOUTTMP
/* 68 +  1 */  lsrs rOUTTMP, rOUTDATA, #8
/* 69 +  1 */  ands rOUTTMP, rPINMASK
bitbang_7:
/*  0 +  2 */  strh rOUTTMP, [rGPIO, #GPIO_ODR]
/*  2 + 59 */  bitbang_pulse_end_59_cycles 7
/* 61 +  9 */  delay rOUTTMP, 9
bitbang_6:
/*  0 + 70 */  bitbang_pulse_70_cycles 6
bitbang_5:
/*  0 + 70 */  bitbang_pulse_70_cycles 5
bitbang_4:
/*  0 + 70 */  bitbang_pulse_70_cycles 4
bitbang_3:
/*  0 + 70 */  bitbang_pulse_70_cycles 3
bitbang_2:
/*  0 + 70 */  bitbang_pulse_70_cycles 2
bitbang_1:
/*  0 + 59 */  bitbang_pulse_61_cycles 1
/* 61 +  1 */  nop
/* 62 +  1 */  nop
/* 63 +  1 */  nop
/* 64 +  1 */  movs rOUTTMP, #2
/* 65 +  1 */  add rBUFFER, rOUTTMP
/* 66 + 3+1*/  b bitbang_0

bitbang_end:
delay_big rTMP, 14400
/* pop r4-r11 */
pop {r4-r7}
mov r8, r4
mov r9, r5
mov r10, r6
mov r11, r7
pop {r4-r7}
bx lr
	.size	bitbang, .-bitbang

