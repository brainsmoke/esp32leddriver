
acc   = 0x00
sp    = 0x02

clkmd = 0x03
ihrcr = 0x0b
misc  = 0x3b

pa    = 0x10
pac   = 0x11
paph  = 0x12

ACC_ZERO_FLAG = 0

CLKMD_ENABLE_ILRC = 1<<2
CLKMD_ENABLE_IHRC = 1<<4
CLKMD_IHRC_DIV2   = 1<<5
CLKMD_IHRC_DIV4   = 0
CLKMD_IHRC_DIV8   = (1<<5) | (1<<3)
CLKMD_IHRC_DIV16  = (0<<5) | (1<<3)
TYPE_IHRC         = 1

MISC_16384        = 2

CLKMD_ENABLE_WATCHDOG = 1<<1

.macro clock_8mhz
	mov a, #(CLKMD_ENABLE_ILRC|CLKMD_ENABLE_IHRC|CLKMD_IHRC_DIV2)
	mov clkmd, a
.endm

.macro clock_4mhz
	mov a, #(CLKMD_ENABLE_ILRC|CLKMD_ENABLE_IHRC|CLKMD_IHRC_DIV4)
	mov clkmd, a
.endm

.macro clock_2mhz
	mov a, #(CLKMD_ENABLE_ILRC|CLKMD_ENABLE_IHRC|CLKMD_IHRC_DIV8)
	mov clkmd, a
.endm

.macro clock_1mhz
	mov a, #(CLKMD_ENABLE_ILRC|CLKMD_ENABLE_IHRC|CLKMD_IHRC_DIV16)
	mov clkmd, a
.endm

.macro watchdog_enable
	mov a, #MISC_16384
	mov misc, a
	mov a, #CLKMD_ENABLE_WATCHDOG
	xor clkmd, a
.endm

; filler pattern that will be replaced with calibration code by the easypdk programmer
.macro easypdk_calibrate frequency, millivolt
	.irp b, 'R', 'C', TYPE_IHRC, frequency, frequency>>8, frequency>>16, frequency>>24, millivolt, millivolt>>8, ihrcr
	and a, #b
	.endm
.endm


