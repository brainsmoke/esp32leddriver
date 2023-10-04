
#include <ws2801par.h>

.include "m0delay.S"

	.cpu cortex-m0
	.fpu softvfp
	.section	.bss
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.align	2
	.global	bitbang_ws2801
	.code	16
	.thumb_func
	.type	bitbang_ws2801, %function
#define rBUF      r0
#define rGPIO     r1
#define rCLK      r2

/* ORDER MATTERS FOR LDM */
#define rCARRY    r3
#define rOUT      r4
#define rTMP      r5
#define rIN0      r6
#define rIN1      r7

#define rBITS_A   r8
#define rBITS_B   r9
#define rBITS_C   r10
#define rBUFEND   r11

bitbang_ws2801: // (rBUF, r1:bytes_per_strip, rCLK, r3:GPIO)
	.syntax unified
	.thumb

push {r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
push {r4-r7}

# mov rBUF, r0 (same)
mov rBUFEND, rBUF
lsls r1, #2
add rBUFEND, r1
lsls r1, #2
add rBUFEND, r1 // rBUFEND = (intptr_t)rBUF + 4*5*size
mov rGPIO, r3

.macro apply_carry reg

	eors \reg, rCARRY
	bics rCARRY, \reg
	lsls rCARRY, #16
	eors \reg, rCARRY
	bics rCARRY, \reg
	lsls rCARRY, #16

.endm

cmp rBUF, rBUFEND
bne 1f
b bitbang_ws2801_end
1:

ldm rBUF!, {rCARRY, rOUT, rTMP, rIN0, rIN1}
apply_carry rOUT
apply_carry rTMP
apply_carry rIN0
apply_carry rIN1
mov rBITS_A, rTMP
mov rBITS_B, rIN0
mov rBITS_C, rIN1

cmp rBUF, rBUFEND
bne 1f
b bitbang_ws2801_epilogue
1:

bitbang_ws2801_loop_start:
/*   BIT 7  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 4 */ ldm rBUF!, {rCARRY, rIN0, rIN1}
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 1 */ eors rIN0, rCARRY
/*    9 + 1 */ bics rCARRY, rIN0
/*   10 + 1 */ lsrs rCARRY, #16
/*   11 + 1 */                                            lsrs rOUT, #16

/*   BIT 6  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */ eors rIN0, rCARRY
/*    3 + 1 */ bics rCARRY, rIN0  
/*    4 + 1 */ lsls rCARRY, #16
/*    5 + 1 */                                            mov rOUT, rBITS_A
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 1 */ mov rBITS_A, rBITS_C
/*    9 + 1 */ eors rIN1, rCARRY
/*   10 + 1 */ bics rCARRY, rIN1
/*   11 + 1 */ lsrs rCARRY, #16

/*   BIT 5  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */ eors rIN1, rCARRY
/*    3 + 1 */ bics rCARRY, rIN1  
/*    4 + 1 */ lsls rCARRY, #16
/*    5 + 1 */                                            lsrs rTMP, rOUT, #16
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 1 */                                            mov rOUT, rBITS_B
/*    9 + 1 */ mov rBITS_C, rIN0  
/*   10 + 1 */ mov rBITS_B, rIN1
/*   11 + 1 */ nop

/*   BIT 4  */
/*    0 + 2 */                                            strh rTMP, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 3 */ ldm rBUF!, {rIN0, rIN1}
/*    5 + 1 */ eors rIN0, rCARRY
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 1 */ bics rCARRY, rIN0
/*    9 + 1 */ lsrs rCARRY, #16
/*   10 + 1 */ eors rIN0, rCARRY
/*   11 + 1 */ bics rCARRY, rIN0  


/*   BIT 3  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */ lsls rCARRY, #16
/*    3 + 1 */ eors rIN1, rCARRY
/*    4 + 1 */ bics rCARRY, rIN1
/*    5 + 1 */ lsrs rCARRY, #16
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 1 */ eors rIN1, rCARRY
/*    9 + 1 */ bics rCARRY, rIN1
/*   10 + 1 */                                            lsrs rTMP, rOUT, #16
/*   11 + 1 */                                            mov rOUT, rBITS_A

/*   BIT 2  */
/*    0 + 2 */                                            strh rTMP, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */ mov rBITS_A, rIN0
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 3 */ delay_simple 3
/*   10 + 1 */                                            lsrs rTMP, rOUT, #16

/*   BIT 1  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */ mov rOUT, rIN1
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 3 */ delay_simple 3
/*   10 + 1 */ cmp rBUF, rBUFEND 

/*   BIT 0  */
/*    0 + 2 */                                            strh rTMP, [rGPIO, #GPIO_ODR_OFFSET]
delay_simple EXTRA_CYCLES_PRE_CLK
/*    2 + 1|4 */ beq bitbang_ws2801_loop_exit
/*    3 + 3 */ delay_simple 3
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ b bitbang_ws2801_loop_start

bitbang_ws2801_loop_exit:
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ b bitbang_ws2801_loop_start


bitbang_ws2801_epilogue:
/*   BIT 7  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */                                            lsrs rOUT, #16
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4

/*   BIT 6  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */                                            mov rOUT, rBITS_A
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4

/*   BIT 5  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */                                            lsrs rOUT, #16
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4

/*   BIT 4  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */                                            mov rOUT, rBITS_B
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4

/*   BIT 3  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */                                            lsrs rOUT, #16
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4

/*   BIT 2  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */                                            mov rOUT, rBITS_C
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4

/*   BIT 1  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    2 + 1 */                                            lsrs rOUT, #16
/*    3 + 3 */ delay_simple 3
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4

/*   BIT 0  */
/*    0 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*    3 + 3 */ delay_simple 4
delay_simple EXTRA_CYCLES_PRE_CLK
/*    6 + 2 */                                            str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
delay_simple EXTRA_CYCLES_POST_CLK
/*    8 + 4 */ delay_simple 4
/*   11 + 1 */ movs rOUT, #0
/*   12 + 2 */                                            strh rOUT, [rGPIO, #GPIO_ODR_OFFSET]
bitbang_ws2801_end:

pop {r4-r7}
mov r8,  r4
mov r9,  r5
mov r10, r6
mov r11, r7
pop {r4-r7}
bx lr
	.size	bitbang_ws2801, .-bitbang_ws2801
#undef rBUF
#undef rGPIO
#undef rCLKMASK
#undef rCARRY
#undef rOUT
#undef rTMP
#undef rIN0
#undef rIN1
#undef rBITS_A
#undef rBITS_B
#undef rBITS_C
#undef rBUFEND



	.global	precomp_dithering
	.code	16
	.thumb_func
	.type	precomp_dithering, %function
	.syntax unified
	.thumb
precomp_dithering: // ( rBUF, rLOWBUF, rRESBUF, rVALUES_PER_STRIP )

// data bits:  _BA9876__543210_
// clock bits: X______XX______X

#if CLK_MASK != 0x8181
#error "CLK_MASK incompatible with asm code"
#endif

#define rBUF                r0
#define rLOBUF              r1
#define rRESBUF             r2
#define rVALUES_PER_STRIP   r3
#define rCARRY              r4
#define rLO                 r5
#define rRES                r6

precomp_start:
push {r4-r6}

b precomp_loop
precomp_end:
pop {r4-r6}
bx lr


precomp_loop:
subs rVALUES_PER_STRIP, #1
blt precomp_end

.macro do_temp_dither offset
ldrb rLO,  [rLOBUF, #\offset]
ldrb rRES, [rRESBUF, #\offset]
adds rRES, rLO
strb rRES, [rRESBUF, #\offset]
lsls rRES, #24
adcs rCARRY, rCARRY
.endm

movs rCARRY, #0
do_temp_dither 11
do_temp_dither 10
do_temp_dither 9
do_temp_dither 8
do_temp_dither 7
do_temp_dither 6
lsls rCARRY, #2
do_temp_dither 5
do_temp_dither 4
do_temp_dither 3
do_temp_dither 2
do_temp_dither 1
do_temp_dither 0
lsls rCARRY, #17

str rCARRY, [rBUF, #0]

adds rLOBUF, #12
adds rRESBUF, #12
adds rBUF, #20

b precomp_loop

	.size	precomp_dithering, .-precomp_dithering
#undef rBUF
#undef rLOBUF
#undef rRESBUF
#undef rVALUES_PER_STRIP
#undef rCARRY
#undef rLO
#undef rRES



	.global	write_value
	.code	16
	.thumb_func
	.type	write_value, %function
	.syntax unified
	.thumb
write_value: // ( rT, rPIN, tCHAR )

#define rT      r0
#define rPIN    r1
#define rCHAR   r2
#define rTMP    r3

#define BIT0 (6)
#define BIT1 (4)
#define BIT2 (10)
#define BIT3 (8)
#define BIT4 (14)
#define BIT5 (12)
#define BIT6 (18)
#define BIT7 (16)

.macro set_bit offset
lsrs rCHAR, #1
bcc 1f
ldrh rTMP, [rT, #\offset]
orrs rTMP, rPIN
strh rTMP, [rT, #\offset]
1:
.endm

set_bit BIT0
set_bit BIT1
set_bit BIT2
set_bit BIT3
set_bit BIT4
set_bit BIT5
set_bit BIT6
set_bit BIT7

bx lr
	.size	write_value, .-write_value
#undef rBUF
#undef rLOBUF
#undef rRESBUF
#undef rVALUES_PER_STRIP
#undef rCARRY
#undef rLO
#undef rRES


