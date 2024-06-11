
#include <timing_experiment.h>

.include "m0delay.S"
	.cpu cortex-m0
	.fpu softvfp
	.section	.bss
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


bitbang: // ( buffer, size, t1h, t1l )
/* push r4-r11 */

push {r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
push {r4-r7}

#define rBUFFER     r0
#define rTMP        r1
#define rGPIO       r2
#define rOUTTMP     r3
#define rOUTDATA    r4
#define rPINMASK    r5

#define rT1H     r8
#define rT1L     r9
#define rBUFEND r11

bitbang_start:

mov rBUFEND, rBUFFER
add rBUFEND, r1
lsls r2, #3
mov rT1H, r2
lsls r3, #3
mov rT1L, r3
ldr rGPIO, =GPIOA_ODR
movs rPINMASK, #1
lsls rPINMASK, #(PIN)

.macro bitbang_pulse_start_4_cycles bit
/* 0 + 2 */  strh rPINMASK, [rGPIO]
/* 2 + 1 */  lsrs rOUTTMP, rOUTDATA, #(\bit)
/* 3 + 1 */  ands rOUTTMP, rPINMASK
.endm

.macro bitbang_pulse_end
/* 10     +     3    */  delay_reg_div8_setup_3cycles rT1H, rTMP, (15), 0
/* 13     +     2    */  strh rOUTTMP, [rGPIO]
/* 15     +  T1H-15  */  delay_reg_div8_actual rTMP
/* T1H    +     2    */  strh rTMP, [rGPIO]
/* T1H+2  +  T1L-2   */  delay_reg_div8 rT1L, rTMP, (2), 0
.endm

.macro load_outdata_4_cycles
/* 0 + 2 */  ldrb rOUTDATA, [rBUFFER]
/* 2 + 1 */  lsls rOUTDATA, #(PIN)
/* 3 + 1 */  adds rBUFFER, #1
.endm

.macro bitbang_pulse bit
bitbang_pulse_start_4_cycles \bit
/* 4 + 6 */ delay rTMP, 6
bitbang_pulse_end
.endm

load_outdata_4_cycles
b bitbang_7

bitbang_epilogue:
/*  ? + ?   */ bitbang_pulse_end
b bitbang_end


bitbang_1_cycle13:
//nop looks like we lose one cycle in the jump, probably due to pipeline stall
nop
nop
/* 10 + ?   */  bitbang_pulse_end
bitbang_0:

/*  0 + 4   */  bitbang_pulse_start_4_cycles 0
/*  4 + 1   */  cmp rBUFFER, rBUFEND
/*  5 + 1|3 */  beq bitbang_epilogue
/*  6 + 4   */  load_outdata_4_cycles
/* 10 + ?   */  bitbang_pulse_end

bitbang_7:
bitbang_pulse 7
bitbang_6:
bitbang_pulse 6
bitbang_5:
bitbang_pulse 5
bitbang_4:
bitbang_pulse 4
bitbang_3:
bitbang_pulse 3
bitbang_2:
bitbang_pulse 2
bitbang_1:
bitbang_pulse_start_4_cycles 1
/* 4 + 3 */ b bitbang_1_cycle13
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

