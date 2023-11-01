
#include <ws2812_new.h>

.include "m0delay.S"

	.cpu cortex-m0
	.fpu softvfp
	.section	.bss
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.align	2
	.global	ws2812_asm_apply_dither
	.code	16
	.thumb_func
	.type	ws2812_asm_apply_dither, %function

ws2812_asm_apply_dither: // (frame_t *f, size_t start_byte, size_t end_byte, uint8_t residuals[])
	.syntax unified
	.thumb

push {r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
push {r4-r7}

#define rBUF          r0

#define hiLO_BUF      r8
#define hiRES_BUF     r9
#define hiCARRY_BUF   r10
#define hiBUFEND      r11

mov hiRES_BUF, r3
mov hiBUFEND, rBUF
lsls r2, #4
add hiBUFEND, r2

mov hiLO_BUF, rBUF
ldr r2, =LOW_BYTES_OFFSET
add hiLO_BUF, r2

mov hiCARRY_BUF, rBUF
ldr r2, =OLD_CARRY_OFFSET
add hiCARRY_BUF, r2

lsls r1, #1
add hiCARRY_BUF, r1
lsls r1, #3

add rBUF, r1
add hiRES_BUF, r1
add hiLO_BUF, r1

// rBUF == &f->transpose[i]
// hiBUFEND == &f->transpose[exit_before_this_one]
// hiLO_BUF == &f->low_bytes[i*16]
// hiRES_BUF == residual[i*16]
// hiCARRY_BUF == &f->old_carry[i];


apply_dither_loop:

#define rNEW_CARRY    r2

#define rLO_BUF       r4
#define rRES_BUF      r5
#define rLO           r6
#define rRES          r7

/*   0 + 1 */ movs rNEW_CARRY, #0
/*   1 + 1 */ mov rLO_BUF, hiLO_BUF
/*   2 + 1 */ mov rRES_BUF, hiRES_BUF

.macro do_temp_dither offset
/* 0 + 2 */ ldrb rLO,  [rLO_BUF, #\offset]
/* 2 + 2 */ ldrb rRES, [rRES_BUF, #\offset]
/* 4 + 1 */ adds rRES, rLO
/* 5 + 2 */ strb rRES, [rRES_BUF, #\offset]
/* 7 + 1 */ lsls rRES, #24
/* 8 + 1 */ adcs rNEW_CARRY, rNEW_CARRY
.endm

/*   3 + 9 */ do_temp_dither 15
/*  12 + 9 */ do_temp_dither 14
/*  21 + 9 */ do_temp_dither 13
/*  30 + 9 */ do_temp_dither 12
/*  39 + 9 */ do_temp_dither 11
/*  48 + 9 */ do_temp_dither 10
/*  57 + 9 */ do_temp_dither  9
/*  66 + 9 */ do_temp_dither  8
/*  75 + 9 */ do_temp_dither  7
/*  84 + 9 */ do_temp_dither  6
/*  93 + 9 */ do_temp_dither  5
/* 102 + 9 */ do_temp_dither  4
/* 111 + 9 */ do_temp_dither  3
/* 120 + 9 */ do_temp_dither  2
/* 129 + 9 */ do_temp_dither  1
/* 138 + 9 */ do_temp_dither  0

#undef rRES
#undef rLO
#define rTMP r6
/* 147 + 1 */ movs rTMP, #16
/* 148 + 1 */ add hiRES_BUF, rTMP
/* 149 + 1 */ add hiLO_BUF, rTMP
#undef rTMP

#undef rLO_BUF
#undef rRES_BUF

#define rCARRY_BUF    r1
#define rOLD_CARRY    r3

/* 150 + 1 */ mov rCARRY_BUF, hiCARRY_BUF
/* 151 + 1 */ ldrh rOLD_CARRY, [rCARRY_BUF]
/* 152 + 1 */ strh rNEW_CARRY, [rCARRY_BUF]
/* 153 + 1 */ adds rCARRY_BUF, #2
/* 154 + 1 */ mov hiCARRY_BUF, rCARRY_BUF

#undef rCARRY_BUF

#define rROT r1

/* ORDER MATTERS FOR LDM */
#define rINVBITS67    r4
#define rINVBITS45    r5
#define rINVBITS23    r6
#define rINVBITS01    r7

.macro parallel_alu_inc_dec reg
/* 0 + 1 */ rors \reg, rROT
/* 1 + 1 */ eors \reg, rOLD_CARRY
/* 2 + 1 */ bics rOLD_CARRY, \reg
/* 3 + 1 */ eors \reg, rNEW_CARRY
/* 4 + 1 */ ands rNEW_CARRY, \reg
/* 5 + 1 */ rors \reg, rROT
/* 6 + 1 */ eors \reg, rOLD_CARRY
/* 7 + 1 */ bics rOLD_CARRY, \reg
/* 8 + 1 */ eors \reg, rNEW_CARRY
/* 9 + 1 */ ands rNEW_CARRY, \reg
.endm

/* 155 +  1 */ movs rROT, #16
/* 156 +  5 */ ldm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}
/* 161 +  1 */ subs rBUF, #16
/* 162 + 10 */ parallel_alu_inc_dec rINVBITS01
/* 172 + 10 */ parallel_alu_inc_dec rINVBITS23
/* 182 + 10 */ parallel_alu_inc_dec rINVBITS45
/* 192 + 10 */ parallel_alu_inc_dec rINVBITS67
/* 202 +  5 */ stm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

#undef rROT
#undef rNEW_CARRY
#undef rOLD_CARRY
#undef rINVBITS67
#undef rINVBITS45
#undef rINVBITS23
#undef rINVBITS01

/* 207 +  1 */ cmp rBUF, hiBUFEND
/* 208 +  1 */ bge 1f
/* 211 +  3 */ b apply_dither_loop
1:

pop {r4-r7}
mov r8,  r4
mov r9,  r5
mov r10, r6
mov r11, r7
pop {r4-r7}
bx lr
	.size	ws2812_asm_apply_dither, .-ws2812_asm_apply_dither
#undef rBUF
#undef hiBUFEND
#undef hiLO_BUF
#undef hiRES_BUF
#undef hiCARRY_BUF

    .global write_value
    .code   16
    .thumb_func
    .type   write_value, %function
    .syntax unified
    .thumb
write_value: // ( rBUF, rPIN, tCHAR )

//#define rBUF    r0
//#define rPIN    r1
//#define rCHAR   r2
//#define rTMP    r3
//
//#define BIT0 (6)
//#define BIT1 (4)
//#define BIT2 (10)
//#define BIT3 (8)
//#define BIT4 (14)
//#define BIT5 (12)
//#define BIT6 (18)
//#define BIT7 (16)
//
//
///* ORDER MATTERS FOR LDM */
//#define rINVBITS67    r4
//#define rINVBITS45    r5
//#define rINVBITS23    r6
//#define rINVBITS01    r7
//
//
///* XX +  5 */ ldm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}
//
///* XX +  1 */ lsrs rCHAR, #1
///* XX +  3 */ bcc 1f
///* XX +  X */ bics rINVBITS01, rPIN_HI
//1:
///* XX +  1 */ lsrs rCHAR, #1
///* XX +  3 */ bcc 1f
///* XX +  X */ bics rINVBITS01, rPIN_LO
//1:
//
///* XX +  1 */ subs rBUF, #16
///* XX +  5 */ stm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}
//
//
//
//.macro set_bit offset
//lsrs rCHAR, #1
//bcc 1f
//ldrh rTMP, [rT, #\offset]
//orrs rTMP, rPIN
//strh rTMP, [rT, #\offset]
//1:
//.endm
//
//set_bit BIT0
//set_bit BIT1
//set_bit BIT2
//set_bit BIT3
//set_bit BIT4
//set_bit BIT5
//set_bit BIT6
//set_bit BIT7
//
bx lr
    .size   write_value, .-write_value
#undef rBUF
#undef rLOBUF
#undef rRESBUF
#undef rVALUES_PER_STRIP
#undef rCARRY
#undef rLO
#undef rRES

