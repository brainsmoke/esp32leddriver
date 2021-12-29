
clkmd = 0x03
ihrcr = 0x0b

pa    = 0x10
pac   = 0x11

CLKMD_ENABLE_ILRC = 1<<2
CLKMD_ENABLE_IHRC = 1<<4
CLKMD_IHRC_DIV2   = 1<<5
TYPE_IHRC         = 1

.macro clock_8mhz
	mov a, #(CLKMD_ENABLE_ILRC|CLKMD_ENABLE_IHRC|CLKMD_IHRC_DIV2)
	mov clkmd, a
.endm

; filler pattern that will be replaced with calibration code by the easypdk programmer
.macro easypdk_calibrate frequency, millivolt
	.irp b, 'R', 'C', TYPE_IHRC, frequency, frequency>>8, frequency>>16, frequency>>24, millivolt, millivolt>>8, ihrcr
	and a, #b
	.endm
.endm


