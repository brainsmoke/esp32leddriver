

/*
 * Main loop in asm:
 *
 * - to prevent the compiler from causing the stack to grow into .bss
 * - to read data from the DMA buffer fast enough, within the 280us timeout
 *
 */

#include "fsm.h"
#include "util.h"
#include "relayball_init.h"
#include "bitbang16_relay_4strip.h"

#define PERIPH_BASE         (                      0x40000000 )
#define AHBPERIPH_BASE      ( PERIPH_BASE        + 0x00020000 )
#define DMA1_BASE           ( AHBPERIPH_BASE     + 0x00000000 )
#define DMA1_CHANNEL3_BASE  ( DMA1_BASE          + 0x00000030 )
#define DMA1_CHANNEL3_CNDTR ( DMA1_CHANNEL3_BASE + 0x00000004 )

#define SCS_BASE            (                      0xE000E000 )
#define SYSTICK_BASE        ( SCS_BASE           + 0x00000010 )
#define SYSTICK_LOAD        ( SYSTICK_BASE       + 0x00000004 )

	.cpu cortex-m0
	.fpu softvfp
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.balign	2
	.global	main
	.code	16
	.thumb_func
	.type	main, %function
	.syntax unified
	.thumb

main:

bl init

#define rSCRATCH  r0
#define rI        r1
#define rPREAD    r2
#define rDATA     r3
#define rNEXT     r4
#define rSTATE    r5
#define rBUFEND   r6
#define rPCNT     r7

#define rFF00     r8
#define rFRAMESZ  r9
#define rCUR      r10
#define rFSM      r11

ldr rSCRATCH, =pulse_width8x
ldr rDATA, =(8*70)
strh rDATA, [rSCRATCH]

ldr r0, =(FRAME_BYTE_COUNT)
muls r0, rDATA
ldr rDATA, =(WS2812TIMEOUT-8) /* systick reset value == period-1 */
add r0, rDATA
lsrs r0, #3 /* systick clock is processor clock/8 */

bl enable_sys_tick

ldr rPREAD, =recv_buf
ldr rBUFEND, =(recv_buf+RECV_BUF_SZ)
ldr rPCNT, =DMA1_CHANNEL3_CNDTR

ldr rNEXT, =frame_a
ldr rSCRATCH, =frame_b
mov rCUR, rSCRATCH

movs rSCRATCH, #0xff
lsls rSCRATCH, #8
mov rFF00, rSCRATCH

ldr rSCRATCH, =N_BYTES
mov rFRAMESZ, rSCRATCH

ldr rSCRATCH, =fsm
mov rFSM, rSCRATCH

next_frame:
	ldr rSCRATCH, =cur
	str rNEXT, [rSCRATCH]
	mov rSCRATCH, rNEXT
	mov rNEXT, rCUR
	mov rCUR, rSCRATCH

ignore_frame:

	movs rI, #0
	read_loop:
		0:	ldr rSCRATCH, [rPCNT]
			subs rSCRATCH, rBUFEND, rSCRATCH
			cmp rSCRATCH, rPREAD
			beq 0b

	    ldrb rDATA, [rPREAD]

		adds rPREAD, #1
		cmp rPREAD, rBUFEND
		bne 1f
			ldr rPREAD, =recv_buf
		1:

		0:	ldr rSCRATCH, [rPCNT]
			subs rSCRATCH, rBUFEND, rSCRATCH
			cmp rSCRATCH, rPREAD
			beq 0b

	    ldrb rSCRATCH, [rPREAD]
		lsls rSCRATCH, #8
		orrs rDATA, rSCRATCH

		adds rPREAD, #1
		cmp rPREAD, rBUFEND
		bne 1f
			ldr rPREAD, =recv_buf
		1:

		cmp rDATA, rFF00
			bgt frame_end

		strh rDATA, [rNEXT, rI]
		adds rI, #2

		cmp rI, rFRAMESZ
			blt read_loop

frame_end:

	movs rSCRATCH, #0
	zerofill_loop:
		cmp rI, rFRAMESZ
		beq read_end_of_frame_marker
		strh rSCRATCH, [rNEXT, rI]
		adds rI, #2
		b zerofill_loop

read_end_of_frame_marker:

	movs rSTATE, #GOOD

	cmp rDATA, rFF00
	ble 1f
		movs rSTATE, #GOOD_FFFF
		uxtb rDATA, rDATA

		cmp rDATA, #0xff
		beq 1f
			movs rSTATE, #BAD_FF
	1:

fsm_loop:

	0:	ldr rSCRATCH, [rPCNT]
		subs rSCRATCH, rBUFEND, rSCRATCH
		cmp rSCRATCH, rPREAD
		beq 0b

    ldrb rDATA, [rPREAD]

	adds rPREAD, #1
	cmp rPREAD, rBUFEND
	bne 1f
		ldr rPREAD, =recv_buf
	1:

	adds rSTATE, #IN_00
	cmp rDATA, #0x00
		beq 1f

	adds rSTATE, #(IN_01_EF-IN_00)
	cmp rDATA, #0xEF
		ble 1f

	adds rSTATE, #(IN_F0-IN_01_EF)
	cmp rDATA, #0xF0
		ble 1f

	adds rSTATE, #(IN_F1-IN_F0)
	cmp rDATA, #0xF1
		ble 1f

	adds rSTATE, #(IN_F2-IN_F1)
	cmp rDATA, #0xF2
		ble 1f

	adds rSTATE, #(IN_F3_FE-IN_F2)
	cmp rDATA, #0xFE
		ble 1f

	adds rSTATE, #(IN_FF-IN_F3_FE)

	1:

	add rSTATE, rFSM
	ldrb rSTATE, [rSTATE]
	cmp rSTATE, #GOOD_RETURN
	beq next_frame
	cmp rSTATE, #BAD_RETURN
	beq ignore_frame
	cmp rSTATE, #ROUTE
	beq new_route
	cmp rSTATE, #TIMING
	beq timing

	b fsm_loop

timing:

#define rPULSEWIDTH     rDATA  /* re-use registers for out of scope variables */
#define rOLDPULSEWIDTH  rI
#define rSYSTICKRESET   rSTATE

	ldrh rPULSEWIDTH, [rNEXT, #0]
	ldr rSCRATCH, =(60*8)
	cmp rPULSEWIDTH, rSCRATCH
		bgt 1f
		mov rPULSEWIDTH, rSCRATCH
	1:

	ldr rSCRATCH, =(90*8)
	cmp rPULSEWIDTH, rSCRATCH
		blt 1f
		mov rPULSEWIDTH, rSCRATCH
	1:

	ldr rSCRATCH, =pulse_width8x
	ldrh rOLDPULSEWIDTH, [rSCRATCH]
	cmp rPULSEWIDTH, rOLDPULSEWIDTH
	bgt 1f                                /* lengthen pulse: set systick first */
	strh rPULSEWIDTH, [rSCRATCH]          /* shorten pulse: set pulse width first */
	1:
#unset rOLDPULSEWIDTH
#define rSCRATCH2      rI

	ldr rSCRATCH2, =(FRAME_BYTE_COUNT)
	muls rSCRATCH2, rPULSEWIDTH
	ldr rSYSTICKRESET, =(WS2812TIMEOUT-8) /* systick reset value == period-1 */
	add rSYSTICKRESET, rSCRATCH2
	lsrs rSYSTICKRESET, #3                /* systick clock is processor clock/8 */
	ldr rSCRATCH2, =SYSTICK_LOAD
	str rSYSTICKRESET, [rSCRATCH2]

	strh rPULSEWIDTH, [rSCRATCH]          /* set pulse width */

b ignore_frame

new_route:

movs rI, #0
0:
	ldr rSCRATCH, =0x80808080
	ldr rDATA, [rNEXT, rI]
	ands rDATA, rSCRATCH
	bne ignore_frame
	adds rI, #4
	cmp rI, #ROUTING_TABLE_SIZE
	blt 0b

ldr rSCRATCH, =routing_table
str rNEXT, [rSCRATCH]

ldr rSCRATCH, =table

movs rI, #0
0:
	ldr rDATA, [rNEXT, rI]
	str rDATA, [rSCRATCH, rI]
	adds rI, #4
	cmp rI, #ROUTING_TABLE_SIZE
	blt 0b

ldr rI, =routing_table
str rSCRATCH, [rI]

b ignore_frame
	.size	main, .-main

