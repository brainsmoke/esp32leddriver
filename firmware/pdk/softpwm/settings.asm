
.macro read_settings mem
ldsptl
mov mem, a
.endm

.macro read_settings_to_a
ldsptl
.endm

.macro next_settings ptr
izsn ptr
dec ptr+1
inc ptr+1
.endm

.macro find_settings label, ptr, ?find_settings_loop, ?found_settings
mov a, #<label
mov ptr, a
mov a, #>label
mov ptr+1, a
mov a, #ptr
mov sp, a
find_settings_loop:
ldspth
ceqsn a, #0
goto found_settings
next_settings ptr
goto find_settings_loop
found_settings:
.endm

