
#include <ws2812_dma16.h>

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


    .global dma_getchar
    .code   16
    .thumb_func
    .type   dma_getchar, %function
    .syntax unified
    .thumb
dma_getchar:
#define rSCRATCH      r0
#define rPREAD        r1
#define rPCNT         r2
#define rDMA_IN_P     r3
ldr rDMA_IN_P, =dma_in_p
ldr rPREAD, [rDMA_IN_P]
#unset rDMA_IN_P
#define rDMA_END      r3
ldr rDMA_END, =(recv_buf + RECV_BUF_SZ)
cmp rPREAD, rDMA_END
bne 1f
    ldr rPREAD, =recv_buf
1:
ldr rPCNT, =DMA_CHANNEL_CNDTR
0:  ldr rSCRATCH, [rPCNT]
    subs rSCRATCH, rDMA_END, rSCRATCH
    cmp rSCRATCH, rPREAD
    beq 0b
ldrb rSCRATCH, [rPREAD]
adds rPREAD, #1

#unset rDMA_END
#define rDMA_IN_P r3
ldr rDMA_IN_P, =dma_in_p
str rPREAD, [rDMA_IN_P]

#undef rSCRATCH
#undef rPREAD
#undef rPCNT
#unset rDMA_IN_P

bx lr
    .size   dma_getchar, .-dma_getchar


    .global asm_read_frame
    .code   16
    .thumb_func
    .type   asm_read_frame, %function
    .syntax unified
    .thumb
asm_read_frame: // ( frame_t *next )

push {r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
push {r4-r7}

#define rDATA         r0
#define rFRAME        r1
#define rPIN          r2
#define rLO_BUF       r3

#define rPREAD        r4
#define rPCNT         r5
#define rSCRATCH      r7

#define hiFRAME_END   r8
#define hiPWRITTEN    r9
#define hiDMA_END     r10
#define hiFF00        r11

mov rFRAME, r0
movs rPIN, #1

ldr rLO_BUF, =LOW_BYTES_OFFSET
add rLO_BUF, rFRAME

ldr rSCRATCH, =dma_in_p
ldr rPREAD, [rSCRATCH]

ldr rSCRATCH, =FRAME_SIZE
mov hiFRAME_END, rSCRATCH
add hiFRAME_END, rFRAME

mov hiPWRITTEN, rPREAD

ldr rSCRATCH, =(recv_buf + RECV_BUF_SZ)
mov hiDMA_END, rSCRATCH

ldr rSCRATCH, =#0xFF00
mov hiFF00, rSCRATCH

push {rLO_BUF}
b asm_read_frame_loop
asm_read_frame_exit:
pop {rLO_BUF}
ldr rSCRATCH, =dma_in_p
str rPREAD, [rSCRATCH]
pop {r4-r7}
mov r8,  r4
mov r9,  r5
mov r10, r6
mov r11, r7
pop {r4-r7}
bx lr

dma_wait_a:

//                      ldr rPCNT, =#0x48000014
//                      ldr rSCRATCH, [rPCNT]
//                      mvns rSCRATCH, rSCRATCH
//                      str rSCRATCH, [rPCNT]
/*  0 + 1 */ cmp rPREAD, hiDMA_END
/*  1 + 3 */ bne 1f
/*  2 + 2 */ ldr rPREAD, =recv_buf
1:
/*  4 + 2 */ ldr rPCNT, =DMA_CHANNEL_CNDTR
/*  6 + 2 */ 0:  ldr rSCRATCH, [rPCNT]
/*  8 + 1 */     rsbs rSCRATCH, rSCRATCH, #0
/*  9 + 1 */     add rSCRATCH, hiDMA_END
/* 10 + 1 */     cmp rSCRATCH, rPREAD
/* 11 + 1 */     beq 0b
/* 12 + 3 */     bgt 1f
/* 13 + 1 */     mov rSCRATCH, hiDMA_END
1:
/* 15 + 1 */     mov hiPWRITTEN, rSCRATCH
//                      ldr rPCNT, =#0x48000014
//                      ldr rSCRATCH, [rPCNT]
//                      mvns rSCRATCH, rSCRATCH
//                      str rSCRATCH, [rPCNT]
b dma_wait_a_ret

dma_wait_b:

//                      ldr rPCNT, =#0x48000014
//                      ldr rSCRATCH, [rPCNT]
//                      mvns rSCRATCH, rSCRATCH
//                      str rSCRATCH, [rPCNT]
/*  0 + 1 */ cmp rPREAD, hiDMA_END
/*  1 + 3 */ bne 1f
/*  2 + 2 */ ldr rPREAD, =recv_buf
1:
/*  4 + 2 */ ldr rPCNT, =DMA_CHANNEL_CNDTR
/*  6 + 2 */ 0:  ldr rSCRATCH, [rPCNT]
/*  8 + 1 */     rsbs rSCRATCH, rSCRATCH, #0
/*  9 + 1 */     add rSCRATCH, hiDMA_END
/* 10 + 1 */     cmp rSCRATCH, rPREAD
/* 11 + 1 */     beq 0b
/* 12 + 3 */     bgt 1f
/* 13 + 1 */     mov rSCRATCH, hiDMA_END
1:
/* 15 + 1 */     mov hiPWRITTEN, rSCRATCH
//                      ldr rPCNT, =#0x48000014
//                      ldr rSCRATCH, [rPCNT]
//                      mvns rSCRATCH, rSCRATCH
//                      str rSCRATCH, [rPCNT]
b dma_wait_b_ret

asm_read_frame_loop:
/*  0 + 1 */ cmp rPREAD, hiPWRITTEN
/*  1 + 1 */ beq dma_wait_a
dma_wait_a_ret:
/*  2 + 2 */ ldrb rDATA, [rPREAD]
/*  4 + 1 */ adds rPREAD, #1
/*  5 + 1 */ cmp rPREAD, hiPWRITTEN
/*  6 + 1 */ beq dma_wait_b
dma_wait_b_ret:
/*  7 + 2 */ ldrb rSCRATCH, [rPREAD]
/*  9 + 1 */ adds rPREAD, #1
/* 10 + 1 */ lsls rSCRATCH, #8
/* 11 + 1 */ add rDATA, rSCRATCH
/* 12 + 1 */ cmp rDATA, hiFF00
/* 13 + 1 */ bgt asm_read_frame_exit
/* 14 + 2 */ strb rDATA, [rLO_BUF]
/* 16 + 1 */ adds rLO_BUF, #16
/* 17 + 3 */ push {rLO_BUF, rPREAD}

#unset rLO_BUF
#unset rPREAD
#unset rPCNT
#unset rSCRATCH
#define rPIN_HI r3
/* 20 + 1 */ lsls rPIN_HI, rPIN, #16

/* ORDER MATTERS FOR LDM */
#define rINVBITS67    r4
#define rINVBITS45    r5
#define rINVBITS23    r6
#define rINVBITS01    r7

/* 21 +  5 */ ldm rFRAME!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

/* 26 +  1 */ lsrs rDATA, #9
/* 27 +  3 */ bcc 1f
/*         */ bics rINVBITS01, rPIN_HI
1:
/* 30 +  1 */ lsrs rDATA, #1
/* 31 +  3 */ bcc 1f
/*         */ bics rINVBITS01, rPIN
1:

/* 34 +  1 */ lsrs rDATA, #1
/* 35 +  3 */ bcc 1f
/*         */ bics rINVBITS23, rPIN_HI
1:
/* 38 +  1 */ lsrs rDATA, #1
/* 39 +  3 */ bcc 1f
/*         */ bics rINVBITS23, rPIN
1:

/* 42 +  1 */ lsrs rDATA, #1
/* 43 +  3 */ bcc 1f
/*         */ bics rINVBITS45, rPIN_HI
1:
/* 46 +  1 */ lsrs rDATA, #1
/* 47 +  3 */ bcc 1f
/*         */ bics rINVBITS45, rPIN
1:

/* 50 +  1 */ lsrs rDATA, #1
/* 51 +  3 */ bcc 1f
/*         */ bics rINVBITS67, rPIN_HI
1:
/* 54 +  1 */ lsrs rDATA, #1
/* 55 +  3 */ bcc 1f
/*         */ bics rINVBITS67, rPIN
1:

/* 58 */
/* 58 +  1 */ subs rFRAME, #16
/* 59 +  5 */ stm rFRAME!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

#unset rINVBITS67
#unset rINVBITS45
#unset rINVBITS23
#unset rINVBITS01

#define rLO_BUF       r3
#define rPREAD        r4
/* 64 + 3  */ pop {rLO_BUF, rPREAD}

/* 67 +  1 */ cmp rFRAME, hiFRAME_END
/* 68 +  3 */ blt asm_read_frame_loop

/* amort...*/ ldr rFRAME, =-FRAME_SIZE
              add rFRAME, hiFRAME_END
              lsls rPIN, #1
              pop {rLO_BUF}
              adds rLO_BUF, #1
              push {rLO_BUF}
              cmp rPIN, hiFF00 // we have this constant already anyway, use as: if ( (1<<x) > (1<<15) )
              blt asm_read_frame_loop
b asm_read_frame_exit
    .size   asm_read_frame, .-asm_read_frame

#undef rDATA
#undef rFRAME
#undef rPIN
#undef rLO_BUF

#undef rPREAD
#undef rPCNT
#undef rSCRATCH

#undef hiFRAME_END
#undef hiPWRITTEN
#undef hiDMA_END
#undef hiFF00


