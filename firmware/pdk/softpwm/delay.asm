;

.macro switchcase_filler n
.rept n
nop
.endm
.endm

.macro delay n, ?l1

.ifgt n - 3*255+2
	.error 1
.else
	mov a, #(n / 3)
	l1:
	dzsn a
	goto l1

	.ifgt n%3
		nop
		.ifgt n%3-1
			nop
		.endif
	.endif
.endif
 
.endm

