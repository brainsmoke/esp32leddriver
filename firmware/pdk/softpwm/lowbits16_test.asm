.module lowbits16_test

BUFSIZE=8

.include "pdk.asm"
.include "delay.asm"
.include "settings.asm"
;.include "uart2.asm"

.include "softpwm16.asm"

.area DATA (ABS)
.org 0x00


; bitwise ops need to have an address < 16

shiftreg:       .ds 1
error:          .ds 1

uart_state:     .ds 1
pin_mask_cur:   .ds 1

pinmask:        .ds 1
uart_tmp:       .ds 1

low1_staging:   .ds 1
low2_staging:   .ds 1
low3_staging:   .ds 1

refresh:        .ds 1

; word aligned
;...
p_lo:           .ds 1
p_hi:           .ds 1

out0:           .ds 1
out1:           .ds 1
cycle:          .ds 1
switch_channel: .ds 1

high:           .ds 1

high1:          .ds 1
high2:          .ds 1
high3:          .ds 1

high1_staging:  .ds 1
high2_staging:  .ds 1
high3_staging:  .ds 1

low:            .ds 1

low1:           .ds 1
low2:           .ds 1
low3:           .ds 1

dith:           .ds 1

dith1:          .ds 1
dith2:          .ds 1
dith3:          .ds 1

dith1_staging:  .ds 1
dith2_staging:  .ds 1
dith3_staging:  .ds 1

cmp1:           .ds 1
cmp2:           .ds 1
cmp3:           .ds 1

rem1:           .ds 1
rem2:           .ds 1
rem3:           .ds 1

low_highnib:    .ds 1
low_cur:        .ds 1
;
index:          .ds 1
cur_channel:    .ds 1

wait_count:     .ds 1
bit_count:      .ds 1
reset_count:    .ds 1

index_const:    .ds 1

test_counter:   .ds 1

.area CODE (ABS)
.org 0x00

; pull mosfets low first

clock_8mhz

;
;   ________                                                                        ________
;           ________0000000011111111222222223333333344444444555555556666666677777777
;                                                                                      /\
;           |-----------| (start_wait+1/2)*interval                                   9 1/2 baud
;        time to first sample (avg)                                               sample stop bit (avg)
;
;baud = 38400
;start_wait = 4
;bit_wait = 3
;check_interval = 64
;cycles = (.5+start_wait+8*bit_wait)*check_interval
;t = 9.5/baud
;freq = cycles/t
;print(freq)
FREQ=7372800

easypdk_calibrate FREQ, 3300

mov a, #255
mov pac, a
mov a, #0
mov pa, a

softpwm_init

mov a, #(1<<2)       ; 11 + 1
mov out1, a          ; 12 + 1

mov a, #33 + 1
mov test_counter, a

set1 pa, #2

loop:

.macro uart
set1 pa, #0                  ;  0 + 1
t1sn pin_mask_cur, #CHANNEL1 ;  1 + 1
inc test_counter             ;  2 + 1
mov a, #33                   ;  3 + 1
dzsn test_counter            ;  4 + 1
mov a, #0                    ;  5 + 1
add test_counter, a          ;  6 + 1
mov a, test_counter          ;  7 + 1
mov low_cur, a               ;  8 + 1
;inc low_cur                 ;  9 + 1
nop                          ;  9 + 1
sl low_cur                   ; 10 + 1
mov a, #(1<<1)               ; 11 + 1
mov out0, a                  ; 12 + 1
set0 pa, #0                  ; 13 + 1
.endm

softpwm_lowbits loop

