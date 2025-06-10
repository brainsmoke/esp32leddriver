
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

ws2812_asm_apply_dither: // (frame_t *f, size_t start_byte, size_t end_byte, residual_state_t residuals[])
	.syntax unified
	.thumb

push {r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
push {r4-r7}

#define hiBUF         r8
#define hiBUFEND      r9
#define hiLO_BUF      r10
#define hiRES_BUF     r11

mov hiBUF, r0
mov hiRES_BUF, r3
mov hiBUFEND, hiBUF

lsls r1, #1
add hiRES_BUF, r1
lsls r1, #3
add hiRES_BUF, r1
add hiBUF, r1

lsls r2, #4
add hiBUFEND, r2

mov hiLO_BUF, hiBUF
ldr r3, =LOW_BYTES_OFFSET
add hiLO_BUF, r3

// hiBUF == &f->transpose[i]
// hiBUFEND == &f->transpose[exit_before_this_one]
// hiLO_BUF == &f->low_bytes[i*16]
// hiRES_BUF == residual[i*16]

apply_dither_loop:

.macro parallel32_half_adder_alu a b carry // a += b
/*   0 +  1 */ eors \a, \b
/*   1 +  1 */ bics \b, \a
/*   2 +  1 */ mov \carry, \b
.endm

.macro parallel32_full_adder_alu a b carry // a += b
/*   0 +  1 */ eors \a, \b
/*   1 +  1 */ bics \b, \a
/*   2 +  1 */ eors \a, \carry
/*   3 +  1 */ bics \carry, \a
/*   4 +  1 */ orrs \carry, \b
.endm

#define rLO_BUF       r0
#define rRES_BUF      r1
#define rA0           r2
#define rB0           r3
#define rB1           r4
#define rB2           r5
#define rB3           r6
#define rNEW_CARRY    r7

/*   0 +  1 */ mov rLO_BUF, hiLO_BUF
/*   1 +  1 */ mov rRES_BUF, hiRES_BUF

/*   2 +  5 */ ldm rLO_BUF!, {rB0, rB1, rB2, rB3}
/*   7 +  2 */ ldr rA0, [rRES_BUF, #0]
/*   9 +  3 */ parallel32_half_adder_alu rA0, rB0, rNEW_CARRY

#undef rB0
#define rA1 r3

/*  12 +  2 */ ldr rA1, [rRES_BUF, #4]
/*  14 +  5 */ parallel32_full_adder_alu rA1, rB1, rNEW_CARRY

#undef rB1
#define rA2 r4

/*  19 +  2 */ ldr rA2, [rRES_BUF, #8]
/*  21 +  5 */ parallel32_full_adder_alu rA2, rB2, rNEW_CARRY

#undef rB2
#define rA3 r5

/*  26 +  2 */ ldr rA3, [rRES_BUF, #12]
/*  28 +  5 */ parallel32_full_adder_alu rA3, rB3, rNEW_CARRY
/*  33 +  5 */ stm rRES_BUF!, {rA0, rA1, rA2, rA3}

#undef rB3
#undef rA0
#undef rA1
#undef rA2
#undef rA3

#define rA4           r2
#define rB4           r3
#define rB5           r4
#define rB6           r5
#define rB7           r6

/*  38 +  5 */ ldm rLO_BUF!, {rB4, rB5, rB6, rB7}

/*  43 +  2 */ ldr rA4, [rRES_BUF, #0]
/*  45 +  5 */ parallel32_full_adder_alu rA4, rB4, rNEW_CARRY

#undef rB4
#define rA5 r3

/*  50 +  2 */ ldr rA5, [rRES_BUF, #4]
/*  52 +  5 */ parallel32_full_adder_alu rA5, rB5, rNEW_CARRY

#undef rB5
#define rA6 r4

/*  57 +  2 */ ldr rA6, [rRES_BUF, #8]
/*  59 +  5 */ parallel32_full_adder_alu rA6, rB6, rNEW_CARRY

#undef rB6
#define rA7 r5

/*  64 +  2 */ ldr rA7, [rRES_BUF, #12]
/*  66 +  5 */ parallel32_full_adder_alu rA7, rB7, rNEW_CARRY

#undef rB7
#define rOLD_CARRY r6

/*  71 +  2 */ ldr rOLD_CARRY, [rRES_BUF, #16]
/*  73 +  6 */ stm rRES_BUF!, {rA4, rA5, rA6, rA7, rNEW_CARRY}

#undef rA4
#undef rA5
#undef rA6
#undef rA7

/*  79 +  1 */ mov hiLO_BUF, rLO_BUF
/*  80 +  1 */ mov hiRES_BUF, rRES_BUF

#undef rLO_BUF
#undef rRES_BUF

#define rBUF r0
/*  81 +  1 */ mov rBUF, hiBUF

# rOLD_CARRY == r6
# rNEW_CARRY == r7

/*  82 +  3 */ push {rOLD_CARRY, rNEW_CARRY}
/*  85 +  1 */ uxth rNEW_CARRY, rNEW_CARRY
/*  86 +  1 */ uxth rOLD_CARRY, rOLD_CARRY

#define rROT r1

/* ORDER MATTERS FOR LDM */
#define rINVBITS67    r2
#define rINVBITS45    r3
#define rINVBITS23    r4
#define rINVBITS01    r5

.macro parallel16_alu_inc_dec reg
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

/*  87 +  1 */ movs rROT, #16
/*  88 +  5 */ ldm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}
/*  93 +  1 */ subs rBUF, #16
/*  94 + 10 */ parallel16_alu_inc_dec rINVBITS01
/* 104 + 10 */ parallel16_alu_inc_dec rINVBITS23
/* 114 + 10 */ parallel16_alu_inc_dec rINVBITS45
/* 124 + 10 */ parallel16_alu_inc_dec rINVBITS67
/* 134 +  5 */ stm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

/* 139 +  3 */ pop {rOLD_CARRY, rNEW_CARRY}
/* 142 +  1 */ lsrs rOLD_CARRY, #16
/* 143 +  1 */ lsrs rNEW_CARRY, #16

/* 144 +  5 */ ldm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}
/* 149 +  1 */ subs rBUF, #16
/* 150 + 10 */ parallel16_alu_inc_dec rINVBITS01
/* 160 + 10 */ parallel16_alu_inc_dec rINVBITS23
/* 170 + 10 */ parallel16_alu_inc_dec rINVBITS45
/* 180 + 10 */ parallel16_alu_inc_dec rINVBITS67
/* 190 +  5 */ stm rBUF!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

#undef rROT
#undef rNEW_CARRY
#undef rOLD_CARRY
#undef rINVBITS67
#undef rINVBITS45
#undef rINVBITS23
#undef rINVBITS01

/* 195 +  1 */ mov hiBUF, rBUF
/* 196 +  1 */ cmp rBUF, hiBUFEND
/* 197 +  1 */ bge 1f
/* 198 +  3 */ b apply_dither_loop
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

#define rFRAME        r0
#define rDATA         r1
#define rPIN          r2
#define rPIN_HI       r3

#define rPREAD        r4
#define rDATA2        r7
#define rSCRATCH0     r5
#define rSCRATCH1     r6

#define hiFRAME_END   r8
#define hiPWRITTEN    r9
#define hiDMA_END     r10
#define hiFF00        r11

#define hiPREAD       r12

//mov rFRAME, r0 (nop)
movs rPIN, #1

ldr rSCRATCH0, =dma_in_p
ldr rSCRATCH0, [rSCRATCH0]
mov rPREAD, rSCRATCH0
mov hiPWRITTEN, rPREAD

ldr rSCRATCH0, =FRAME_SIZE
mov hiFRAME_END, rSCRATCH0
add hiFRAME_END, rFRAME

ldr rSCRATCH0, =(recv_buf + RECV_BUF_SZ)
mov hiDMA_END, rSCRATCH0

ldr rSCRATCH0, =#0xFF00
mov hiFF00, rSCRATCH0

b asm_read_frame_loop
asm_read_frame_exit:
ldr rSCRATCH0, =dma_in_p
str rPREAD, [rSCRATCH0]
mov r0, rDATA
pop {r4-r7}
mov r8,  r4
mov r9,  r5
mov r10, r6
mov r11, r7
pop {r4-r7}
bx lr

.macro pin_high scratch0, scratch1, pin
	ldr \scratch0, =#0x48000018
	ldr \scratch1, =#( 1 << \pin )
	strh \scratch1, [ \scratch0 ]
.endm

.macro pin_low scratch0, scratch1, pin
	ldr \scratch0, =#0x48000028
	ldr \scratch1, =#( 1 << \pin )
	strh \scratch1, [ \scratch0 ]
.endm


.macro dma_wait reg_p_cndtr, reg_dma_end

//pin_high \reg_p_cndtr, \reg_dma_end, DEBUG_PIN

/*  0 + 1 */ cmp rPREAD, hiDMA_END
/*  1 + 3 */ bne 1f
/*  2 + 2 */ ldr rPREAD, =recv_buf
1:
/*  4 + 2 */ ldr \reg_p_cndtr, =DMA_CHANNEL_CNDTR
/*  6 + 2 */ 0:  ldr \reg_dma_end, [\reg_p_cndtr]
/*  8 + 1 */     rsbs \reg_dma_end, \reg_dma_end, #0
/*  9 + 1 */     add \reg_dma_end, hiDMA_END
/* 10 + 1 */     cmp \reg_dma_end, rPREAD
/* 11 + 1 */     beq 0b
/* 12 + 3 */     bgt 1f
/* 13 + 1 */     mov \reg_dma_end, hiDMA_END
1:
/* 15 + 1 */     mov hiPWRITTEN, \reg_dma_end

//pin_low \reg_p_cndtr, \reg_dma_end, DEBUG_PIN

.endm

dma_wait_a:
dma_wait rSCRATCH0, rSCRATCH1
b dma_wait_a_ret

dma_wait_b:
dma_wait rSCRATCH0, rSCRATCH1
b dma_wait_b_ret

dma_wait_c:
dma_wait rSCRATCH0, rSCRATCH1
b dma_wait_c_ret

dma_wait_d:
dma_wait rSCRATCH0, rSCRATCH1
b dma_wait_d_ret

asm_read_frame_loop:

// read low byte
/*  0 + 1 */ cmp rPREAD, hiPWRITTEN
/*  1 + 1 */ beq dma_wait_a
dma_wait_a_ret:
/*  2 + 2 */ ldrb rDATA, [rPREAD]
/*  4 + 1 */ adds rPREAD, #1

// read high byte
/*  5 + 1 */ cmp rPREAD, hiPWRITTEN
/*  6 + 1 */ beq dma_wait_b
dma_wait_b_ret:
/*  7 + 2 */ ldrb rSCRATCH0, [rPREAD]
/*  9 + 1 */ adds rPREAD, #1

// merge
/* 10 + 1 */ lsls rSCRATCH0, #8
/* 11 + 1 */ add rDATA, rSCRATCH0
/* 12 + 1 */ cmp rDATA, hiFF00
/* 13 + 1 */ bgt asm_read_frame_exit

// read low byte
/* 14 + 1 */ cmp rPREAD, hiPWRITTEN
/* 15 + 1 */ beq dma_wait_c
dma_wait_c_ret:
/* 16 + 2 */ ldrb rDATA2, [rPREAD]
/* 18 + 1 */ adds rPREAD, #1

// read high byte
/* 19 + 1 */ cmp rPREAD, hiPWRITTEN
/* 20 + 1 */ beq dma_wait_d
dma_wait_d_ret:
/* 21 + 2 */ ldrb rSCRATCH0, [rPREAD]
/* 23 + 1 */ adds rPREAD, #1

// merge
/* 24 + 1 */ lsls rSCRATCH0, #8
/* 25 + 1 */ add rDATA2, rSCRATCH0
/* 26 + 1 */ cmp rDATA2, hiFF00
/* 27 + 1 */ bgt asm_read_frame_exit

// merge 2 values, (val2<<16) || (val1)
/* 28 + 1 */ lsls rDATA2, #16
/* 29 + 1 */ add rDATA, rDATA2

/* 30 + 1 */ mov hiPREAD, rPREAD
/* 31 + 2 */ ldr rSCRATCH0, =LOW_BYTES_OFFSET
/* 33 + 1 */ add rFRAME, rSCRATCH0

#unset rPREAD
#unset rDATA2
#unset rSCRATCH0
#unset rSCRATCH1

#define rLOWBITS0 r4
#define rLOWBITS1 r5
#define rLOWBITS2 r6
#define rLOWBITS3 r7

/* 34 +  5 */ ldm rFRAME!, {rLOWBITS0, rLOWBITS1, rLOWBITS2, rLOWBITS3}
/* 39 +  1 */ subs rFRAME, #16

/* 40 +  1 */ lsrs rSCRATCH0, rDATA, #1         // A[0]
/* 41 +  3 */ bcc 1f
/*         */ orrs rLOWBITS0, rPIN
1:

/* 44 +  1 */ lsrs rSCRATCH0, rDATA, #2         // A[1]
/* 45 +  3 */ bcc 1f
/*         */ orrs rLOWBITS1, rPIN
1:

/* 48 +  1 */ lsrs rSCRATCH0, rDATA, #3         // A[2]
/* 49 +  3 */ bcc 1f
/*         */ orrs rLOWBITS2, rPIN
1:

/* 52 +  1 */ lsrs rSCRATCH0, rDATA, #4         // A[3]
/* 53 +  3 */ bcc 1f
/*         */ orrs rLOWBITS3, rPIN
1:

/* 56 +  1 */ lsls rPIN, #16

/* 57 +  1 */ lsrs rSCRATCH0, rDATA, #17        // B[0]
/* 58 +  3 */ bcc 1f
/*         */ orrs rLOWBITS0, rPIN
1:

/* 61 +  1 */ lsrs rSCRATCH0, rDATA, #18        // B[1]
/* 62 +  3 */ bcc 1f
/*         */ orrs rLOWBITS1, rPIN
1:

/* 65 +  1 */ lsrs rSCRATCH0, rDATA, #19        // B[2]
/* 66 +  3 */ bcc 1f
/*         */ orrs rLOWBITS2, rPIN
1:

/* 69 +  1 */ lsrs rSCRATCH0, rDATA, #20        // B[3]
/* 70 +  3 */ bcc 1f
/*         */ orrs rLOWBITS3, rPIN
1:

/* 73 +  5 */ stm rFRAME!, {rLOWBITS0, rLOWBITS1, rLOWBITS2, rLOWBITS3}
/* 78 +  5 */ ldm rFRAME!, {rLOWBITS0, rLOWBITS1, rLOWBITS2, rLOWBITS3}
/* 83 +  1 */ subs rFRAME, #16

/* 84 +  1 */ lsrs rSCRATCH0, rDATA, #21        // B[4]
/* 85 +  3 */ bcc 1f
/*         */ orrs rLOWBITS0, rPIN
1:

/* 88 +  1 */ lsrs rSCRATCH0, rDATA, #22        // B[5]
/* 89 +  3 */ bcc 1f
/*         */ orrs rLOWBITS1, rPIN
1:

/* 92 +  1 */ lsrs rSCRATCH0, rDATA, #23        // B[6]
/* 93 +  3 */ bcc 1f
/*         */ orrs rLOWBITS2, rPIN
1:

/* 98 +  1 */ lsrs rSCRATCH0, rDATA, #24        // B[7]
/* 99 +  3 */ bcc 1f
/*         */ orrs rLOWBITS3, rPIN
1:

/*102 +  1 */ lsrs rPIN, #16

/*103 +  1 */ lsrs rSCRATCH0, rDATA, #5         // A[4]
/*104 +  3 */ bcc 1f
/*         */ orrs rLOWBITS0, rPIN
1:

/*107 +  1 */ lsrs rSCRATCH0, rDATA, #6         // A[5]
/*108 +  3 */ bcc 1f
/*         */ orrs rLOWBITS1, rPIN
1:

/*111 +  1 */ lsrs rSCRATCH0, rDATA, #7         // A[6]
/*112 +  3 */ bcc 1f
/*         */ orrs rLOWBITS2, rPIN
1:

/*115 +  1 */ lsrs rSCRATCH0, rDATA, #8         // A[7]
/*116 +  3 */ bcc 1f
/*         */ orrs rLOWBITS3, rPIN
1:
/*119 +  5 */ stm rFRAME!, {rLOWBITS0, rLOWBITS1, rLOWBITS2, rLOWBITS3}

/*124 +  2 */ ldr rSCRATCH0, =(LOW_BYTES_OFFSET+32)
/*126 +  1 */ subs rFRAME, rSCRATCH0

#unset rPREAD
#unset rSCRATCH
#define rPIN_HI r3
/*127 + 1 */ lsls rPIN_HI, rPIN, #16

/* ORDER MATTERS FOR LDM */
#define rINVBITS67    r4
#define rINVBITS45    r5
#define rINVBITS23    r6
#define rINVBITS01    r7

/*128 +  5 */ ldm rFRAME!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

/*133 +  1 */ lsrs rDATA, #9
/*134 +  3 */ bcc 1f
/*         */ bics rINVBITS01, rPIN_HI
1:
/*137 +  1 */ lsrs rDATA, #1
/*138 +  3 */ bcc 1f
/*         */ bics rINVBITS01, rPIN
1:

/*141 +  1 */ lsrs rDATA, #1
/*142 +  3 */ bcc 1f
/*         */ bics rINVBITS23, rPIN_HI
1:
/*145 +  1 */ lsrs rDATA, #1
/*146 +  3 */ bcc 1f
/*         */ bics rINVBITS23, rPIN
1:

/*149 +  1 */ lsrs rDATA, #1
/*150 +  3 */ bcc 1f
/*         */ bics rINVBITS45, rPIN_HI
1:
/*153 +  1 */ lsrs rDATA, #1
/*154 +  3 */ bcc 1f
/*         */ bics rINVBITS45, rPIN
1:

/*157 +  1 */ lsrs rDATA, #1
/*158 +  3 */ bcc 1f
/*         */ bics rINVBITS67, rPIN_HI
1:
/*161 +  1 */ lsrs rDATA, #1
/*162 +  3 */ bcc 1f
/*         */ bics rINVBITS67, rPIN
1:

/*165 +  1 */ subs rFRAME, #16
/*166 +  5 */ stm rFRAME!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

/*171 +  5 */ ldm rFRAME!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

/*176 +  1 */ lsrs rDATA, #9
/*177 +  3 */ bcc 1f
/*         */ bics rINVBITS01, rPIN_HI
1:
/*180 +  1 */ lsrs rDATA, #1
/*181 +  3 */ bcc 1f
/*         */ bics rINVBITS01, rPIN
1:

/*184 +  1 */ lsrs rDATA, #1
/*185 +  3 */ bcc 1f
/*         */ bics rINVBITS23, rPIN_HI
1:
/*188 +  1 */ lsrs rDATA, #1
/*189 +  3 */ bcc 1f
/*         */ bics rINVBITS23, rPIN
1:

/*192 +  1 */ lsrs rDATA, #1
/*193 +  3 */ bcc 1f
/*         */ bics rINVBITS45, rPIN_HI
1:
/*195 +  1 */ lsrs rDATA, #1
/*196 +  3 */ bcc 1f
/*         */ bics rINVBITS45, rPIN
1:

/*199 +  1 */ lsrs rDATA, #1
/*200 +  3 */ bcc 1f
/*         */ bics rINVBITS67, rPIN_HI
1:
/*203 +  1 */ lsrs rDATA, #1
/*204 +  3 */ bcc 1f
/*         */ bics rINVBITS67, rPIN
1:

/*207 +  1 */ subs rFRAME, #16
/*208 +  5 */ stm rFRAME!, {rINVBITS67, rINVBITS45, rINVBITS23, rINVBITS01}

#unset rINVBITS67
#unset rINVBITS45
#unset rINVBITS23
#unset rINVBITS01
#define rPREAD        r4

/*213 +  1 */ mov rPREAD, hiPREAD
/*214 +  1 */ cmp rFRAME, hiFRAME_END
/*215 +  1 */ bge 1f
0:
/*216 +  3 */ b asm_read_frame_loop
1:
/* amort...*/ ldr rFRAME, =-FRAME_SIZE
              add rFRAME, hiFRAME_END
              lsls rPIN, #1
              cmp rPIN, hiFF00 // we have this constant already anyway, use as: if ( (1<<x) > (1<<15) )
              blt 0b
              b asm_read_frame_exit

    .size   asm_read_frame, .-asm_read_frame

#undef rDATA
#undef rFRAME
#undef rPIN
#undef rPREAD

#undef hiFRAME_END
#undef hiPWRITTEN
#undef hiDMA_END
#undef hiFF00

