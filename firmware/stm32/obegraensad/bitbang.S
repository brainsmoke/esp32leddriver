
#include <obegraensad.h>

.include "m0delay.S"

	.cpu cortex-m0
	.fpu softvfp
	.section	.bss
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.align	2

	.global	bitbang64_clk_stm32
	.code	16
	.thumb_func
	.type	bitbang64_clk_stm32, %function
bitbang64_clk_stm32:
	.syntax unified
	.thumb

#define rBUF   r0
#define rGPIO  r1
#define rCLK   r2
#define rOUT   r3
bitbang64_clk_stm32_start:

/*    0 + 2 */ ldm rBUF!, {rOUT}
/*    2 + 1 */ movs rCLK, #(MASK_CLK)
.rept 15
/* +  0 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* +  2 + 1 */ lsrs rOUT, #8
nop
nop
nop
/* +  3 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/* +  5 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* +  7 + 1 */ lsrs rOUT, #8
nop
nop
nop
/* +  8 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/* + 10 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* + 12 + 1 */ lsrs rOUT, #8
nop
nop
nop
/* + 13 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/* + 15 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* + 17 + 2 */ ldm rBUF!, {rOUT}
nop
nop
/* + 19 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop
.endr
/*  318 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  320 + 1 */ lsrs rOUT, #8
nop
nop
nop
/*  321 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/*  323 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  325 + 1 */ lsrs rOUT, #8
nop
nop
nop
/*  326 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/*  328 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  330 + 1 */ lsrs rOUT, #8
nop
nop
nop
/*  331 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/*  333 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
nop
nop
nop
nop
#undef rBUF
#define rTMP   r0
/*  335 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
/*  337 + 1 */ movs rTMP, #(BIT_LATCH|BIT_NOT_OUTPUT_ENABLE)
/*  338 + 1 */ orrs rOUT, rTMP
nop
nop
/*  339 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  341 + 1 */ movs rTMP, #(MASK_DATA|BIT_LATCH|BIT_NOT_OUTPUT_ENABLE)
/*  342 + 1 */ bics rOUT, rTMP
nop
nop
/*  343 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]

bitbang64_clk_stm32_end:
/*  345 + 3 */ bx lr
/*  348     */

#undef rTMP
#undef rCLK
#undef rGPIO
#undef rOUT
	.size	bitbang64_clk_stm32, .-bitbang64_clk_stm32



.macro delay_reg reg scratch sub


                                              //  [reg&3==0] [reg&3==1] [reg&3==2] [reg&3==3]

   mov \scratch, \reg                         //       1          1          1          1
   addsub \scratch, (-(\sub+5))               //       1          1          1          1
   lsrs \scratch, \scratch, #1                //       1          1          1          1
   bcs 1f                                     //       1          3          1          3
   nop                                        //       1                     1
1: lsrs \scratch, \scratch, #1                //       1          1          1          1
   bcs 2f                                     //       1          1          3          3
2: subs \scratch, #1                          //                    reg>>2
   bne 2b                                     //                 3*(reg>>2)-2

.endm


	.global	bitbang64_clk_no_enable_stm32
	.code	16
	.thumb_func
	.type	bitbang64_clk_no_enable_stm32, %function
bitbang64_clk_no_enable_stm32:
	.syntax unified
	.thumb

#define rBUF   r0
#define rGPIO  r1
#define rCLK   r2
#define rOUT   r3
bitbang64_clk_no_enable_stm32_start:

/*    0 + 2 */ ldm rBUF!, {rOUT}
/*    2 + 1 */ movs rCLK, #(MASK_CLK)
.rept 15
/* +  0 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* +  2 + 1 */ lsrs rOUT, #8
nop
nop
nop
/* +  3 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/* +  5 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* +  7 + 1 */ lsrs rOUT, #8
nop
nop
nop
/* +  8 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/* + 10 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* + 12 + 1 */ lsrs rOUT, #8
nop
nop
nop
/* + 13 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/* + 15 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/* + 17 + 2 */ ldm rBUF!, {rOUT}
nop
nop
/* + 19 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop
.endr
/*  318 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  320 + 1 */ lsrs rOUT, #8
nop
nop
nop
/*  321 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/*  323 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  325 + 1 */ lsrs rOUT, #8
nop
nop
nop
/*  326 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/*  328 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  330 + 1 */ lsrs rOUT, #8
nop
nop
nop
/*  331 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
nop
nop
nop
nop

/*  333 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
nop
nop
nop
nop
#undef rBUF
#define rTMP   r0
/*  335 + 2 */ str rCLK, [rGPIO, #GPIO_BSRR_OFFSET]
/*  337 + 1 */ movs rTMP, #(BIT_LATCH|BIT_NOT_OUTPUT_ENABLE)
/*  338 + 1 */ orrs rOUT, rTMP
nop
nop
/*  339 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]
/*  341 + 1 */ movs rTMP, #(MASK_DATA|BIT_LATCH)
/*  342 + 1 */ bics rOUT, rTMP
nop
nop
/*  343 + 2 */ strb rOUT, [rGPIO, #GPIO_ODR_OFFSET]

bitbang64_clk_no_enable_stm32_end:
/*  345 + 3 */ bx lr
/*  348     */

#undef rTMP
#undef rCLK
#undef rGPIO
#undef rOUT
	.size	bitbang64_clk_no_enable_stm32, .-bitbang64_clk_no_enable_stm32

	.global	write_wait_write
	.code	16
	.thumb_func
	.type	write_wait_write, %function
write_wait_write:
	.syntax	unified
	.thumb

cmp r3, #11
bge 11f
cmp r3, #9
bgt 10f
beq 9f
cmp r3, #7
bgt 8f
beq 7f
cmp r3, #5
bgt 6f
beq 5f
cmp r3, #3
bgt 4f
beq 3f
2:
str r1, [r0]
str r2, [r0]
bx lr
3:
str r1, [r0]
delay r3, 1
str r2, [r0]
bx lr
4:
str r1, [r0]
delay r3, 2
str r2, [r0]
bx lr
5:
str r1, [r0]
delay r3, 3
str r2, [r0]
bx lr
6:
str r1, [r0]
delay r3, 4
str r2, [r0]
bx lr
7:
str r1, [r0]
delay r3, 5
str r2, [r0]
bx lr
8:
str r1, [r0]
delay r3, 6
str r2, [r0]
bx lr
9:
str r1, [r0]
delay r3, 7
str r2, [r0]
bx lr
10:
str r1, [r0]
delay r3, 8
str r2, [r0]
bx lr
11:
str r1, [r0]
delay_reg r3, r1, 2
str r2, [r0]
bx lr

	.size	write_wait_write, .-write_wait_write

