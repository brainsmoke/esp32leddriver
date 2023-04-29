DEVICE = PFS154

misc = 0x08

; see PMS150 datasheet 5.10.3, does not apply to pfs540 it seems
.macro brownout_erratum_workaround_init
	nop
	nop
	nop
.endm

.macro brownout_erratum_workaround_2cycles
	nop
	nop
.endm

