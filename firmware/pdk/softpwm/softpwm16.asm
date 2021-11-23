
CHANNEL1  = 4
CHANNEL2  = 3
CHANNEL3  = 6
PINMASK   = ( (1<<CHANNEL1) | (1<<CHANNEL2) | (1<<CHANNEL3) )

REFRESH   = 2 ; should not be the same a any of the channel bits
DATA_OK   = 1 ; should not be the same a any of the channel bits

CASE_CHANNEL_PRE_INSTR=3
CASE_CHANNEL_INSTR=6
CASE_CHANNEL_1=(CASE_CHANNEL_PRE_INSTR+0*CASE_CHANNEL_INSTR)
CASE_CHANNEL_2=(CASE_CHANNEL_PRE_INSTR+1*CASE_CHANNEL_INSTR)
CASE_CHANNEL_3=(CASE_CHANNEL_PRE_INSTR+2*CASE_CHANNEL_INSTR)

PRE_DATA_INSTR=5
POST_DATA_INSTR=3
READ_CHANNEL_1_INSTR=20
READ_CHANNEL_2_INSTR=20
;READ_CHANNEL_3_INSTR=...

PRE_DATA=2
POST_DATA=(PRE_DATA+PRE_DATA_INSTR)
READ_CHANNEL_1=(POST_DATA+POST_DATA_INSTR)
READ_CHANNEL_2=(READ_CHANNEL_1+READ_CHANNEL_1_INSTR)
READ_CHANNEL_3=(READ_CHANNEL_2+READ_CHANNEL_2_INSTR)

.globl INDEX

;
; STATE MACHINE:
;
;

.macro softpwm_init

;	mov a, #0
;	mov pa, a
;	mov a, #PINMASK
;	mov pac, a

	clear refresh
	clear cmp1
	clear cmp2
	clear cmp3
	clear low1
	clear low2
	clear low3
	clear dith1
	clear dith2
	clear dith3
	clear rem1
	clear rem2
	clear rem3
	mov a, #1
	mov high1, a
	mov high2, a
	mov high3, a
	mov cycle, a
	clear pin_mask_cur
	set1 pin_mask_cur, #CHANNEL1
	mov a, #PINMASK
	mov pinmask, a
	mov a, #CASE_CHANNEL_1
	mov switch_channel, a

	mov a, #READ_CHANNEL_1            ; 31 + 1
	mov cur_channel, a                ; 32 + 1
	mov a, index_const                ; 33 + 1
	mov index, a                      ; 34 + 1
	ceqsn a, #0                       ; 35 + 1
	mov a, #(READ_CHANNEL_1-PRE_DATA) ; 36 + 1
	sub cur_channel, a                ; 37 + 1
.endm

.macro softpwm_lowbits exit ?jmp_tgt_A, ?jmp_tgt_B, ?jmp_tgt_C, ?jmp_tgt_1, ?jmp_tgt_29, ?jmp_tgt_30, ?case31_exit, ?case_32_cont, ?cycle_42, ?A1, ?B1, ?C1, ?A2, ?B2, ?C2, ?A3, ?B3, ?C3, ?A4, ?B4, ?C4, ?A5, ?B5, ?C5, ?A6, ?B6, ?C6, ?A7, ?B7, ?C7, ?A8, ?B8, ?C8, ?A9, ?B9, ?C9

	uart                 ; 50 + 14
	mov a, low_cur       ;  0 + 1  ( bits 0:5 << 1 )
	pcadd a              ;  1 + 2
nop; filler
;case_00
	mov a, #0            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_01
	nop                  ;  3 + 1
	goto jmp_tgt_1       ;  4 + 2
;case_02
	mov a, #9            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_03
	mov a, #9            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_04
	mov a, #9            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_05
	mov a, #8            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_06
	mov a, #8            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_07
	mov a, #8            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_08
	mov a, #7            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_09
	mov a, #7            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_10
	mov a, #7            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_11
	mov a, #6            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_12
	mov a, #6            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_13
	mov a, #6            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_14
	mov a, #5            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_15
	mov a, #5            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_16
	mov a, #5            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_17
	mov a, #4            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_18
	mov a, #4            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_19
	mov a, #4            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_20
	mov a, #3            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_21
	mov a, #3            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_22
	mov a, #3            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_23
	mov a, #2            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_24
	mov a, #2            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_25
	mov a, #2            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_26
	mov a, #1            ;  3 + 1
	goto jmp_tgt_C       ;  4 + 2
;case_27
	mov a, #1            ;  3 + 1
	goto jmp_tgt_B       ;  4 + 2
;case_28
	mov a, #1            ;  3 + 1
	goto jmp_tgt_A       ;  4 + 2
;case_29
	mov a, #0            ;  3 + 1
	goto jmp_tgt_29      ;  4 + 2
;case_30
	mov a, #0            ;  3 + 1
	goto jmp_tgt_30      ;  4 + 2
;case_31
	mov a, switch_channel;  3 + 1
	pcadd a              ;  4 + 2
;case 32
	mov a, pin_mask_cur  ;  3 + 1
	goto case_32_cont    ;  4 + 2
	switchcase_filler (CASE_CHANNEL_PRE_INSTR-3)
;case_31_pin1
	mov a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	set1 pa, #CHANNEL1   ;  8 + 1
	goto case31_exit     ;  9 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-4)
;case_31_pin2
	mov a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	set1 pa, #CHANNEL2   ;  8 + 1
	goto case31_exit     ;  9 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-4)
;case_31_pin3
	mov a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	set1 pa, #CHANNEL3   ;  8 + 1
	goto case31_exit     ;  9 + 2
;	switchcase_filler (CASE_CHANNEL_INSTR-4)
case31_exit:
	clear out0           ; 11 + 1
	clear pin_mask_cur   ; 12 + 1
	goto A2              ; 13 + 2

case_32_cont:
	or a, out0           ;  6 + 1
	mov pa, a            ;  7 + 1
	clear pin_mask_cur   ;  8 + 1
	clear out0           ;  9 + 1
	goto A1              ; 10 + 2

jmp_tgt_1:
	mov a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	clear pin_mask_cur   ;  8 + 1
	delay (34-9)         ;  9 + ..
	mov a, switch_channel; 34 + 1
	pcadd a              ; 35 + 2
	switchcase_filler (CASE_CHANNEL_PRE_INSTR-1)
;pin1
	mov a, out1          ; 37 + 1
	set1 pa, #CHANNEL1   ; 38 + 1
	mov pa, a            ; 39 + 1
	goto cycle_42        ; 40 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-4)
;pin2
	mov a, out1          ; 37 + 1
	set1 pa, #CHANNEL2   ; 38 + 1
	mov pa, a            ; 39 + 1
	goto cycle_42        ; 40 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-4)
;pin3
	mov a, out1          ; 37 + 1
	set1 pa, #CHANNEL3   ; 38 + 1
	mov pa, a            ; 39 + 1
	goto cycle_42        ; 40 + 2
;	switchcase_filler (CASE_CHANNEL_INSTR-4)

jmp_tgt_A:
	xch a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	or a, pin_mask_cur   ;  8 + 1
	dzsn out0            ;  9 + 1
	goto A1              ; 10 + 1|2
	mov pa, a            ; 11 + 1        28  A: 1
A1:	dzsn out0            ; 12 + 1
	goto A2              ; 13 + 1|2
	mov pa, a            ; 14 + 1        25  A: 2
A2:	dzsn out0            ; 15 + 1
	goto A3              ; 16 + 1|2
	mov pa, a            ; 17 + 1        22  A: 3
A3:	dzsn out0            ; 18 + 1
	goto A4              ; 19 + 1|2
	mov pa, a            ; 20 + 1        19  A: 4
A4:	dzsn out0            ; 21 + 1
	goto A5              ; 22 + 1|2
	mov pa, a            ; 23 + 1        16  A: 5
A5:	dzsn out0            ; 24 + 1
	goto A6              ; 25 + 1|2
	mov pa, a            ; 26 + 1        13  A: 6
A6:	dzsn out0            ; 27 + 1
	goto A7              ; 28 + 1|2
	mov pa, a            ; 29 + 1        10  A: 7
A7:	dzsn out0            ; 30 + 1
	goto A8              ; 31 + 1|2
	mov pa, a            ; 32 + 1         7  A: 8
A8:	dzsn out0            ; 33 + 1
	goto A9              ; 34 + 1|2
	mov pa, a            ; 35 + 1         4  A: 9
A9:	nop                  ; 36 + 1
	clear pin_mask_cur   ; 37 + 1|2
	mov a, out1          ; 38 + 1
	mov pa, a            ; 39 + 1         0  A: 0
	goto cycle_42        ; 40 + 2


jmp_tgt_B:
	xch a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	or a, pin_mask_cur   ;  8 + 1
	clear pin_mask_cur   ;  9 + 1
	dzsn out0            ; 10 + 1
	goto B1              ; 11 + 1|2
	mov pa, a            ; 12 + 1        27  A: 1
B1:	dzsn out0            ; 13 + 1
	goto B2              ; 14 + 1|2
	mov pa, a            ; 15 + 1        24  A: 2
B2:	dzsn out0            ; 16 + 1
	goto B3              ; 17 + 1|2
	mov pa, a            ; 18 + 1        21  A: 3
B3:	dzsn out0            ; 19 + 1
	goto B4              ; 20 + 1|2
	mov pa, a            ; 21 + 1        18  A: 4
B4:	dzsn out0            ; 22 + 1
	goto B5              ; 23 + 1|2
	mov pa, a            ; 24 + 1        15  A: 5
B5:	dzsn out0            ; 25 + 1
	goto B6              ; 26 + 1|2
	mov pa, a            ; 27 + 1        12  A: 6
B6:	dzsn out0            ; 28 + 1
	goto B7              ; 29 + 1|2
	mov pa, a            ; 30 + 1         9  A: 7
B7:	dzsn out0            ; 31 + 1
	goto B8              ; 32 + 1|2
	mov pa, a            ; 33 + 1         6  A: 8
B8:	dzsn out0            ; 34 + 1
	goto B9              ; 35 + 1|2
	mov pa, a            ; 36 + 1         3  A: 9
B9:	nop                  ; 37 + 1
	mov a, out1          ; 38 + 1
	mov pa, a            ; 39 + 1
	goto cycle_42        ; 40 + 2


jmp_tgt_C:
	xch a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	or a, pin_mask_cur   ;  8 + 1
	clear pin_mask_cur   ;  9 + 1
	nop                  ; 10 + 1
	dzsn out0            ; 11 + 1
	goto C1              ; 12 + 1|2
	mov pa, a            ; 13 + 1        26  A: 1
C1:	dzsn out0            ; 14 + 1
	goto C2              ; 15 + 1|2
	mov pa, a            ; 16 + 1        23  A: 2
C2:	dzsn out0            ; 17 + 1
	goto C3              ; 18 + 1|2
	mov pa, a            ; 19 + 1        20  A: 3
C3:	dzsn out0            ; 20 + 1
	goto C4              ; 21 + 1|2
	mov pa, a            ; 22 + 1        17  A: 4
C4:	dzsn out0            ; 23 + 1
	goto C5              ; 24 + 1|2
	mov pa, a            ; 25 + 1        14  A: 5
C5:	dzsn out0            ; 26 + 1
	goto C6              ; 27 + 1|2
	mov pa, a            ; 28 + 1        11  A: 6
C6:	dzsn out0            ; 29 + 1
	goto C7              ; 30 + 1|2
	mov pa, a            ; 31 + 1         8  A: 7
C7:	dzsn out0            ; 32 + 1
	goto C8              ; 33 + 1|2
	mov pa, a            ; 34 + 1         5  A: 8
C8:	dzsn out0            ; 35 + 1
	goto C9              ; 36 + 1|2
	mov pa, a            ; 37 + 1         2  A: 9
C9:	mov a, out1          ; 38 + 1
	mov pa, a            ; 39 + 1
	goto cycle_42        ; 40 + 2

jmp_tgt_29:
	xch a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	or a, pin_mask_cur   ;  8 + 1
	clear pin_mask_cur   ;  9 + 1
	mov pa, a            ; 10 + 1        29  A: 0
	goto B1              ; 11 + 2

jmp_tgt_30:
	xch a, out0          ;  6 + 1
	mov pa, a            ;  7 + 1
	or a, pin_mask_cur   ;  8 + 1
	mov pa, a            ;  9 + 1        30  A: 0
	clear pin_mask_cur   ; 10 + 1
	goto B1              ; 11 + 2

cycle_42:
	mov a, switch_channel; 42 + 1
	pcadd a              ; 43 + 2
	switchcase_filler (CASE_CHANNEL_PRE_INSTR-1)
; CHANNEL1
	mov a, #CASE_CHANNEL_2        ; 45 + 1
	mov switch_channel, a         ; 46 + 1
	set1 pin_mask_cur, #CHANNEL2  ; 47 + 1
	goto exit                     ; 48 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-4)
; CHANNEL2
	mov a, #CASE_CHANNEL_3        ; 45 + 1
	mov switch_channel, a         ; 46 + 1
	set1 pin_mask_cur, #CHANNEL3  ; 47 + 1
	goto exit                     ; 48 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-4)
; CHANNEL3
	mov a, #CASE_CHANNEL_1        ; 45 + 1
	mov switch_channel, a         ; 46 + 1
	set1 pin_mask_cur, #CHANNEL1  ; 47 + 1
	goto exit                     ; 48 + 2
;	switchcase_filler (CASE_CHANNEL_INSTR-4)

.endm

.macro softpwm ?highbits, ?l1, ?pre_switch, ?l2, ?lowbits, ?cycle_continue, ?pre_data, ?post_data, ?read_channel_1, ?read_channel_2, ?read_channel_3, ?cycle_44, ?cycle_29, ?cycle_38, ?no_new_data, ?idempotent_ops, ?do_reset, ?do_refresh, ?switch_new_data

highbits:
	uart                   ; 50 + 14
	mov a, #PINMASK        ;  0 + 1
	dzsn cmp1              ;  1 + 1
	xor a, #(1<<CHANNEL1)  ;  2 + 1
	dzsn cmp2              ;  3 + 1
	xor a, #(1<<CHANNEL2)  ;  4 + 1
	dzsn cmp3              ;  5 + 1
	xor a, #(1<<CHANNEL3)  ;  6 + 1
	xor pa, a              ;  7 + 1

	dzsn cycle             ;  8 + 1. 
	goto cycle_continue    ;  9 + 2 )

	mov a, switch_channel  ; 10 + 1
	pcadd a                ; 11 + 2
	switchcase_filler (CASE_CHANNEL_PRE_INSTR-1)
; CHANNEL1
	mov a, low1            ; 13 + 1
	mov low_cur, a         ; 14 + 1
	mov a, dith1           ; 15 + 1
	add rem1, a            ; 16 + 1
	addc low_cur           ; 17 + 1
	goto l1                ; 18 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-6) ; should be 0 instructions here
; CHANNEL2
	mov a, low2            ; 13 + 1
	mov low_cur, a         ; 14 + 1
	mov a, dith2           ; 15 + 1
	add rem2, a            ; 16 + 1
	addc low_cur           ; 17 + 1
	goto l1                ; 18 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-6) ; should be 0 instructions here
; CHANNEL3
	mov a, low3            ; 13 + 1
	mov low_cur, a         ; 14 + 1
	mov a, dith3           ; 15 + 1
	add rem3, a            ; 16 + 1
	addc low_cur           ; 17 + 1
	goto l1                ; 18 + 2
; END SWITCH
l1:
	inc low_cur            ; 20 + 1
	sl low_cur             ; 21 + 1
	delay (32-22)          ; 22 + ...
	mov a, #PINMASK        ; 32 + 1
	dzsn cmp1              ; 33 + 1
	xor a, #(1<<CHANNEL1)  ; 34 + 1
	dzsn cmp2              ; 35 + 1
	xor a, #(1<<CHANNEL2)  ; 36 + 1
	dzsn cmp3              ; 37 + 1
	xor a, #(1<<CHANNEL3)  ; 38 + 1
	xor pa, a              ; 39 + 1
	delay (50-40)          ; 40 + ...

pre_switch:
	uart                   ; 50 + 14
	mov a, #PINMASK        ;  0 + 1
	dzsn cmp1              ;  1 + 1
	xor a, #(1<<CHANNEL1)  ;  2 + 1
	dzsn cmp2              ;  3 + 1
	xor a, #(1<<CHANNEL2)  ;  4 + 1
	dzsn cmp3              ;  5 + 1
	xor a, #(1<<CHANNEL3)  ;  6 + 1
	xor pa, a              ;  7 + 1

	nop                    ;  8 + 1 probabaly needed
	mov a, pa              ;  9 + 1 a == pa
	and a, #PINMASK        ; 10 + 1 needed?
	dzsn cmp1              ; 11 + 1
	xor a, #(1<<CHANNEL1)  ; 12 + 1
	dzsn cmp2              ; 13 + 1
	xor a, #(1<<CHANNEL2)  ; 14 + 1
	dzsn cmp3              ; 15 + 1
	xor a, #(1<<CHANNEL3)  ; 16 + 1
	mov out1, a            ; 17 + 1
	dzsn cmp1              ; 18 + 1
	xor a, #(1<<CHANNEL1)  ; 19 + 1
	dzsn cmp2              ; 20 + 1
	xor a, #(1<<CHANNEL2)  ; 21 + 1
	dzsn cmp3              ; 22 + 1
	xor a, #(1<<CHANNEL3)  ; 23 + 1
	mov out0, a            ; 24 + 1
	mov a, #19             ; 25 + 1
	mov cycle, a           ; 26 + 1
	mov a, switch_channel  ; 27 + 1
	pcadd a                ; 28 + 2
	switchcase_filler (CASE_CHANNEL_PRE_INSTR-1)
; CHANNEL1
	dzsn cmp1              ; 30 + 1
	set0 pinmask, #CHANNEL1; 31 + 1
	mov a, high1           ; 32 + 1
	mov cmp1, a            ; 33 + 1
	nop                    ; 34 + 1
	goto l2                ; 35 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-6) ; should be 0 instructions here
; CHANNEL2
	dzsn cmp2              ; 30 + 1
	set0 pinmask, #CHANNEL2; 31 + 1
	mov a, high2           ; 32 + 1
	mov cmp2, a            ; 33 + 1
	nop                    ; 34 + 1
	goto l2                ; 35 + 2
	switchcase_filler (CASE_CHANNEL_INSTR-6) ; should be 0 instructions here
; CHANNEL3
	dzsn cmp3              ; 30 + 1
	set0 pinmask, #CHANNEL3; 31 + 1
	mov a, high3           ; 32 + 1
	mov cmp3, a            ; 33 + 1
	inc cycle              ; 34 + 1
	goto l2                ; 35 + 2
; END SWITCH
l2:	mov a, out1            ; 37 + 1
	xor a, #PINMASK        ; 38 + 1
	mov pa, a              ; 39 + 1
	mov a, out0            ; 40 + 1
	dzsn cmp1              ; 41 + 1
	xor a, #(1<<CHANNEL1)  ; 42 + 1
	dzsn cmp2              ; 43 + 1
	xor a, #(1<<CHANNEL2)  ; 44 + 1
	dzsn cmp3              ; 45 + 1
	xor a, #(1<<CHANNEL3)  ; 46 + 1
	xor a, pinmask         ; 47 + 1
	mov out1, a            ; 48 + 1
	nop                    ; 49 + 1
lowbits:
	softpwm_lowbits highbits

cycle_continue:
	mov a, #PINMASK             ; 11 + 1
	dzsn cmp1                   ; 12 + 1
	xor a, #(1<<CHANNEL1)       ; 13 + 1
	dzsn cmp2                   ; 14 + 1
	xor a, #(1<<CHANNEL2)       ; 15 + 1
	dzsn cmp3                   ; 16 + 1
	xor a, #(1<<CHANNEL3)       ; 17 + 1
	mov out1, a                 ; 18 + 1
	mov a, cur_channel          ; 19 + 1
	t0sn uart_state, #NEW_DATA  ; 20 + 1 .
switch_new_data:
	pcadd a                     ; 21 + 2  )
	goto no_new_data            ; 22 + 2 '
	.ifne (PRE_DATA-2)
	.error 1
	.endif
pre_data:
	mov a, #READ_CHANNEL_1      ; 23 + 1
	dzsn index                  ; 24 + 1
	mov a, #PRE_DATA            ; 25 + 1
	mov cur_channel, a          ; 26 + 1
	goto cycle_29               ; 27 + 2
	.ifne (PRE_DATA_INSTR-5)
	.error 1
	.endif
post_data:
	nop2                        ; 23 + 2
	nop2                        ; 25 + 2
	goto cycle_29               ; 27 + 2
	.ifne (POST_DATA_INSTR-3)
	.error 1
	.endif

read_channel_1:
	mov a, #READ_CHANNEL_2      ; 23 + 1
	mov cur_channel, a          ; 24 + 1
	mov a, high                 ; 25 + 1
	mov high1_staging, a        ; 26 + 1
	sr high1_staging            ; 27 + 1
	inc high1_staging           ; 28 + 1
	and low, a                  ; 29 + 1
	and a, #1                   ; 30 + 1
	or a, low_highnib           ; 31 + 1
	swap a                      ; 32 + 1
	inc low                     ; 33 + 1
	addc a                      ; 34 + 1
	mov low1_staging, a         ; 35 + 1
	t0sn low1_staging, #5       ; 36 + 1
	clear dith                  ; 37 + 1
	mov a, out1                 ; 38 + 1
	xor pa, a                   ; 39 + 1
	mov a, dith                 ; 40 + 1
	mov dith1_staging, a        ; 41 + 1
	goto cycle_44               ; 42 + 2
	.ifne (READ_CHANNEL_1_INSTR-20)
	.error 1
	.endif

read_channel_2:
	mov a, #READ_CHANNEL_3      ; 23 + 1
	mov cur_channel, a          ; 24 + 1
	mov a, high                 ; 25 + 1
	mov high2_staging, a        ; 26 + 1
	sr high2_staging            ; 27 + 1
	inc high2_staging           ; 28 + 1
	and low, a                  ; 29 + 1
	and a, #1                   ; 30 + 1
	or a, low_highnib           ; 31 + 1
	swap a                      ; 32 + 1
	inc low                     ; 33 + 1
	addc a                      ; 34 + 1
	mov low2_staging, a         ; 35 + 1
	t0sn low2_staging, #5       ; 36 + 1
	clear dith                  ; 37 + 1
	mov a, out1                 ; 38 + 1
	xor pa, a                   ; 39 + 1
	mov a, dith                 ; 40 + 1
	mov dith2_staging, a        ; 41 + 1
	goto cycle_44               ; 42 + 2
	.ifne (READ_CHANNEL_2_INSTR-20)
	.error 1
	.endif

read_channel_3:
	mov a, #POST_DATA           ; 23 + 1
	mov cur_channel, a          ; 24 + 1
	mov a, high                 ; 25 + 1
	mov high3_staging, a        ; 26 + 1
	sr high3_staging            ; 27 + 1
	inc high3_staging           ; 28 + 1
	and low, a                  ; 29 + 1
	and a, #1                   ; 30 + 1
	or a, low_highnib           ; 31 + 1
	swap a                      ; 32 + 1
	inc low                     ; 33 + 1
	addc a                      ; 34 + 1
	mov low3_staging, a         ; 35 + 1
	t0sn low3_staging, #5       ; 36 + 1
	clear dith                  ; 37 + 1
	mov a, out1                 ; 38 + 1
	xor pa, a                   ; 39 + 1
	mov a, dith                 ; 40 + 1
	mov dith3_staging, a        ; 41 + 1
	mov a, #(1<<DATA_OK)        ; 42 + 1
	mov refresh, a              ; 43 + 1
cycle_44:
	nop2                        ; 44 + 2
	nop                         ; 46 + 1
	set0 uart_state, #NEW_DATA  ; 47 + 1
	goto highbits               ; 48 + 2
cycle_29:
	delay (38-29)               ; 29 + ..
cycle_38:
	mov a, out1                 ; 38 + 1
	xor pa, a                   ; 39 + 1
	nop2                        ; 40 + 2
	goto cycle_44               ; 42 + 2

no_new_data:
	t0sn refresh, #REFRESH          ; 24 + 1
	goto do_refresh                 ; 25 + 2
	t0sn uart_state, #RESET         ; 26 + 1
	goto do_reset                   ; 27 + 2

idempotent_ops:
	mov a, low             ; 28 + 1 low is written at least 10 samples
	and a, #0xf0           ; 29 + 1 before NEW_DATA is set, ample time
	mov low_highnib, a     ; 30 + 1 for this to run at least once
	xor a, low             ; 31 + 1
	swap a                 ; 32 + 1
	mov dith, a            ; 33 + 1
	mov a, #PINMASK        ; 34 + 1
	mov pinmask, a         ; 35 + 1
	goto cycle_38          ; 36 + 2

do_reset:
	set0 uart_state, #RESET         ; 29 + 1
	clear error                     ; 30 + 1
	mov a, #READ_CHANNEL_1          ; 31 + 1
	mov cur_channel, a              ; 32 + 1
	mov a, index_const              ; 33 + 1
	mov index, a                    ; 34 + 1
	ceqsn a, #0                     ; 35 + 1
	mov a, #READ_CHANNEL_1-PRE_DATA ; 36 + 1
	sub cur_channel, a              ; 37 + 1
	mov a, out1                     ; 38 + 1
	xor pa, a                       ; 39 + 1
	t0sn refresh, #DATA_OK          ; 40 + 1
	set1 refresh, #REFRESH          ; 41 + 1
	goto cycle_44                   ; 42 + 2

do_refresh:
	clear refresh               ; 27 + 1

	mov a, dith1_staging        ; 28 + 1
	mov dith1, a                ; 29 + 1
	mov a, dith2_staging        ; 30 + 1
	mov dith2, a                ; 31 + 1
	mov a, dith3_staging        ; 32 + 1
	mov dith3, a                ; 33 + 1

	mov a, high1_staging        ; 34 + 1
	mov high1, a                ; 35 + 1
	mov a, high2_staging        ; 36 + 1
	mov high2, a                ; 37 + 1

	mov a, out1                 ; 38 + 1
	xor pa, a                   ; 39 + 1

	mov a, high3_staging        ; 40 + 1
	mov high3, a                ; 41 + 1

	mov a, low1_staging         ; 42 + 1
	mov low1, a                 ; 43 + 1
	mov a, low2_staging         ; 44 + 1
	mov low2, a                 ; 45 + 1
	mov a, low3_staging         ; 46 + 1
	mov low3, a                 ; 47 + 1

	goto highbits               ; 48 + 2

.endm

