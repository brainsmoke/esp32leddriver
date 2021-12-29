;
; ( [ ws2812 data pulses ] [ skipped pulse ] [ 4 bit number ] [ 4 bit mask ] )*
;
;  0         1                   ?                   ,              `
;  0        .______________. _ _ _ _ _ _ _ _ _ .                   ._____
;  0        |1             .     ?             .     ,             |`
;___________|1             . _ _ _ _ _ _ _ _ _ .___________________|`
;  0         1                   ?                   ,              `
;  0.______________. _ _ _ _ _ _ _ _ _ .                   .__________
;  0|        1     .             ?     .             ,     |        `
;___|        1     . _ _ _ _ _ _ _ _ _ .___________________|        `
;  0         1                   ?                   ,              `
; _ _ _ .    1              .______________. _ _ _ _ _ _ _ _ _ .    `
;  0    .    1              |    ?         .         ,         .    `
; _ _ _ .___________________|    ?         . _ _ _ _ _ _ _ _ _ .__________.
;  0         1                   ?                   ,              `
;  .    .    .    .    .    .    .    .    .    .    .    .    .    .    .
; -2   -1    0    1    2    3    4    5    6    7    8    9   10   11   12
;
;  t0sn pa, bit (read 0)
;  .    <skip>
;  .    .    t0sn pa, bit (read 1)
;  .    .    .    goto bit_start
;  .    .    .    .    .    mov pa, a
;  .    .    .    .    .    .    t0sn pa, bit
;  .    .    .    .    .    .    .    <skip>
;  .    .    .    .    .    .    .    .    xor pa, a
;  .    .    .    .    .    .    .    .    .    <wait 4 cycles>
;  .    .    .    .    .    .    .    .    .    .    .    .    .    t0sn pa, bit ...
;
;  .    .    .    .    .    .    .    goto read_one
;  .    .    .    .    .    .    .    .    .    nop
;  .    .    .    .    .    .    .    .    .    .    t0sn pa, bit (read 0, confirm it's a waveform)
;  .    .    .    .    .    .    .    .    .    .    .    <skip>
;  .    .    .    .    .    .    .    .    .    .    .    .    xor pa, a
;  .    .    .    .    .    .    .    .    .    .    .    .    .    t0sn pa, bit ...
;

.module ledrelay

.include "pdk.asm"

PIN_A = 7
PIN_B = 6
PIN_C = 3
PIN_D = 4
PIN_E = 0

RESET_COUNT_HIGH = 16
RESET_COUNT_LOW = 16

PIN_MASK = ((1<<PIN_A)|(1<<PIN_B)|(1<<PIN_C)|(1<<PIN_D)|(1<<PIN_E))

.area DATA (ABS)
.org 0x00

count_low:     .ds 1
count_high:    .ds 1
count_forward: .ds 1
tmp:           .ds 1

.area CODE (ABS)
.org 0x00

	.macro init

		mov a, #0
		mov pa, a
		mov a, #~PIN_MASK
		mov pac, a
		clear count_low
		clear count_high
		clear count_forward
		clear tmp

	.endm

	.macro reset_timeout ?reset_loop, ?outer_loop, ?inner_loop

        reset_loop:              ; cycles = 1+high*4+high*low*6
		mov a, #RESET_COUNT_HIGH
		mov count_high, a
		outer_loop:

			mov a, #RESET_COUNT_LOW
			mov count_low, a
			inner_loop:
				mov a, pa
				ceqsn a, #0
				goto reset_loop
				dzsn count_low
				goto inner_loop

			dzsn count_high
			goto outer_loop

	.endm

	.macro wait_for_data ?loop, ?l1, ?l2, ?l3, ?l4, ?l5

		loop:
			mov a, pa
			mov tmp, a
			.rept 9
			mov a, pa
			or tmp, a
			.endm
			mov a, pa
			or a, tmp
			clear tmp
			and a, #PIN_MASK
			ceqsn a, #0
			goto l1
			goto loop
		l1:	ceqsn a, #(1<<PIN_A)
			goto l2
			goto a_input
		l2:	ceqsn a, #(1<<PIN_B)
			goto l3
			goto b_input
		l3:	ceqsn a, #(1<<PIN_C)
			goto l4
			goto c_input
		l4:	ceqsn a, #(1<<PIN_D)
			goto l5
			goto d_input
		l5:	ceqsn a, #(1<<PIN_E)
			goto timeout          ; data from multiple nodes
			goto e_input
	.endm

	.macro wait_for_pulse IN, n, out
		.rept n
		t0sn pa, #IN
		goto out
		.endm
		goto reset
	.endm

	.macro msg_data_1 IN, ifone, always, ?end
		mov pa, a                          ;  3
		t0sn pa, #IN                       ;  4
		ifone                              ;   ) 5
		nop                                ;  6
        always                             ;  7
		t0sn pa, #IN                       ;  8
		goto reset                         ;   )
		xor pa, a                          ; 10
		t0sn pa, #IN
		goto end
		t1sn pa, #IN
		goto reset
		end:
	.endm

	.macro msg_data_0 IN, ifone, always, ?end
		mov pa, a                          ;  3
		t0sn pa, #IN                       ;  4
		ifone                              ;   ) 5
		xor pa, a                          ;  6
        always                             ;  7
		t0sn pa, #IN                       ;  8
		goto reset                         ;   )
		nop                                ; 10
		t0sn pa, #IN
		goto end
		t1sn pa, #IN
		goto reset
		end:
	.endm

	.macro msg_data_0_last IN, OUT
		mov pa, a                          ;  3
		t0sn pa, #IN                       ;  4
		set1 pac, #OUT                     ;   ) 5
		xor pa, a                          ;  6
		mov a, pac                         ;  7
		and a, #PIN_MASK                   ;  8
      ; goto x
	.endm


	.macro relay IN, OUT1, OUT2, OUT3, OUT4, ?sync, ?sync_loop, ?bit_start, ?read_one, ?l1, ?l2, ?l3, ?route_data, ?bit_fast_start
			mov a, #0
			clear count_forward
			inc count_forward

			sync_loop:
			t0sn pa, #IN
			goto sync_loop
			sync:
			wait_for_pulse IN, 10, bit_start

			read_one:
			nop                  ; 7                4
			t0sn pa, #IN         ; 8                5
			goto reset           ;  )               6
			xor pa, a            ; 10               7 out: pulse low ( 1 bit )

		l1:	t0sn pa, #IN
			goto bit_start
			t1sn pa, #IN
			goto l3

			bit_start:
			mov pa, a            ; 3                0 out: pulse high
			t0sn pa, #IN         ; 4                1
			goto read_one        ;  ) 5             2
			xor pa, a            ; 6                3 out: pulse low ( 0 bit )
			goto l2              ; 7                4,5
		l2:	goto l1              ; 9 --> 11 l1:     6,7

			route_data:
			msg_data_1 IN, ^/set1 count_forward, #3/, nop
			msg_data_1 IN, ^/set1 count_forward, #2/, nop
			msg_data_1 IN, ^/set1 count_forward, #1/, nop
			msg_data_1 IN, ^/set1 count_forward, #0/, ^/inc count_forward/
			msg_data_0 IN, ^/set1 pac, #OUT4/, nop
			msg_data_0 IN, ^/set1 pac, #OUT3/, nop
			msg_data_0 IN, ^/set1 pac, #OUT2/, nop
			msg_data_0_last IN, OUT1
			goto l1

        l3: dzsn count_forward
			goto sync
			wait_for_pulse #IN, 10, route_data
	.endm

clock_8mhz
easypdk_calibrate 8000000, 5000

reset:

init

timeout:
reset_timeout

wait_for_data

a_input:
relay PIN_A, PIN_B, PIN_C, PIN_D, PIN_E

b_input:
relay PIN_B, PIN_C, PIN_D, PIN_E, PIN_A

c_input:
relay PIN_C, PIN_D, PIN_E, PIN_A, PIN_B

d_input:
relay PIN_D, PIN_E, PIN_A, PIN_B, PIN_C

e_input:
relay PIN_E, PIN_A, PIN_B, PIN_C, PIN_D

