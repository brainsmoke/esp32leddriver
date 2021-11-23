
PIN_UART   = 0
MASK_UART  = 1<<(PIN_UART)
START_DELAY = 4
DATA_DELAY = 3
RESET_TIMEOUT = 64

;state flags
IDLE=0
NEW_DATA=1
LOW_BYTE=2
RESET=3

.macro nop2 ?l1
	goto l1
	l1:
.endm

.macro uart_init
	mov a, #MASK_UART
	mov paph, a
	clear error
	mov a, #( (1<<IDLE) | (1<<RESET) )
	mov uart_state, a
	clear shiftreg
	clear high
	clear low
	clear bit_count
	clear reset_count
	clear wait_count
	clear uart_tmp
.endm

.macro uart ?exit, ?l_waitidle, ?l_sample, ?l_highread, ?l_idle, ?l_idle_nodata, ?l_idle_noreset, ?l_wait, ?l_sample

	dzsn wait_count                    ;  0 + 1 --.
	goto l_waitidle                    ;  1 + 2   |
	dzsn bit_count                     ;  2 + 1 <-' --.
	goto l_sample                      ;  3 + 2       |
	; stop bit                                        |
	mov a, shiftreg                    ;  4 + 1     <-'
	t1sn pa, #PIN_UART                 ;  5 + 1
	set1 error, #NEW_DATA              ;  6 + 2
	t0sn uart_state, #LOW_BYTE         ;  7 + 1
	goto l_highread                    ;  8 + 2
	mov low, a                         ;  9 + 1
	mov a, #((1<<LOW_BYTE)|(1<<IDLE))  ; 10 + 1
	mov uart_state, a                  ; 11 + 1
	goto exit                          ; 12 + 2

l_waitidle:
	t1sn uart_state, #IDLE             ;  3 + 1 --.
	goto l_wait                        ;  4 + 2   |
l_idle:                                ;          |
	; uart_state=idle, try read start bit         |
	t0sn pa, #PIN_UART               ;  5 + 1   <-' --.
	goto l_idle_nodata               ;  6 + 2         |
	mov a, #(START_DELAY)            ;  7 + 1       <-'
	mov wait_count, a                ;  8 + 1
	mov a, #9                        ;  9 + 1
	mov bit_count, a                 ; 10 + 1
	set0 uart_state, #IDLE           ; 11 + 1
	goto exit                        ; 12 + 2

l_idle_nodata:
	dzsn reset_count                 ;  8 + 1
	goto l_idle_noreset              ;  9 + 2
	mov a, #((1<<RESET)|(1<<IDLE))   ; 10 + 1
	mov uart_state, a                ; 11 + 1
	goto exit                        ; 12 + 2
	

l_idle_noreset:
	clear wait_count                 ; 11 + 1
	goto exit                        ; 12 + 2

l_wait:
	; not the time to sample, eat cycles
	clear uart_tmp                   ;  6 + 1
	nop                              ;  7 + 1
	nop2                             ;  8 + 2
	mov a, #RESET_TIMEOUT            ; 10 + 1 (idempotent ops)
	mov reset_count, a               ; 11 + 1
	goto exit                        ; 12 + 2


l_sample:
	; sample data
	t0sn pa, #PIN_UART       ;  5 + 1
	set1 uart_tmp, #7        ;  6 + 1
	sr shiftreg              ;  7 + 1
	t0sn uart_tmp, #7        ;  8 + 1
	set1 shiftreg, #7        ;  9 + 1
	mov a, #(DATA_DELAY)     ; 10 + 1
	mov wait_count, a        ; 11 + 1
	goto exit                ; 12 + 2


l_highread:
	mov high, a                        ; 10 + 1
	mov a, #((1<<NEW_DATA)|(1<<IDLE))  ; 11 + 1
	xor a, error                       ; 12 + 1 [erases NEW_DATA bit on error]
	mov uart_state, a                  ; 13 + 1
exit:

.endm

