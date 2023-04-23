
; see PMS150 datasheet 5.10.3
.macro brownout_erratum_workaround_init
	set1 inten, #7
	mov a, #0
	mov intrq, a
.endm

.macro brownout_erratum_workaround_2cycles
	t1sn inten, #7
	reset
.endm

