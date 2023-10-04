
GCC=arm-none-eabi-gcc
CFLAGS=-Wall -g -std=c99 -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb -ffunction-sections -fdata-sections -I platform -I. -Os
LDFLAGS=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT)
LDFLAGS_072=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT_072)

LINKER_SCRIPT=platform/stm32f030.ld
LINKER_SCRIPT_072=platform/stm32f072.ld
STARTUP_SRC=platform/startup_stm32f0xx.s

TARGETS=                  \
	dither8.bin           \
	dither16.bin          \
	strip4.bin            \
	strip5.bin            \
	relayball.bin         \
	uartball16.bin        \
	ws2801par.bin         \
	jitter_timer.bin      \
	timing_experiment.bin

DITHER8_HEADERS=dither8.h util.h m0delay.S
DITHER16_HEADERS=dither16.h util.h m0delay.S
STRIP5_HEADERS=strip5.h util.h m0delay.S
STRIP4_HEADERS=strip4.h util.h fsm.h m0delay.S
RELAYBALL_HEADERS=relayball.h util.h fsm.h m0delay.S
UARTBALL16_HEADERS=uartball16.h util.h
WS2801PAR_HEADERS=ws2801par.h util.h m0delay.S
JITTER_TIMER_HEADERS=jitter_timer.h m0delay.S
TIMING_EXPERIMENT_HEADERS=timing_experiment.h m0delay.S

DITHER8_SRCS=dither8.c dither8_bitbang.S util.c $(STARTUP_SRC)
DITHER16_SRCS=dither16.c dither16_bitbang.S util.c $(STARTUP_SRC)
STRIP4_SRCS=strip4.c strip4_bitbang.S util.c $(STARTUP_SRC)
STRIP5_SRCS=strip5.S strip5_init.c strip5_bitbang.S util.c fsm.c $(STARTUP_SRC)
RELAYBALL_SRCS=relayball.S relayball_init.c relayball_bitbang.S util.c fsm.c $(STARTUP_SRC)
UARTBALL16_SRCS=uartball16.c util.c $(STARTUP_SRC)
WS2801PAR_SRCS=ws2801par.c ws2801par_bitbang.S util.c $(STARTUP_SRC)
JITTER_TIMER_SRCS=jitter_timer.c jitter_timer.S $(STARTUP_SRC)
TIMING_EXPERIMENT_SRCS=timing_experiment.c timing_experiment.S $(STARTUP_SRC)

.PHONY: all clean test

.PRECIOUS: %.elf

all:$(TARGETS)

clean:
	-rm $(TARGETS) *.elf test/*.pyc

test: $(TARGETS)
	test/test_bitbang.py dither8.elf
	test/test_bitbang16.py dither16.elf
	test/test_strip4.py strip4.elf
	test/test_strip5.py strip5.elf
	test/test_relay16.py relayball.elf

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $< $@

dither8.elf: $(DITHER8_SRCS) $(LINKER_SCRIPT) $(DITHER8_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(DITHER8_FLAGS) $(DITHER8_SRCS) -o $@

dither16.elf: $(DITHER16_SRCS) $(LINKER_SCRIPT) $(DITHER16_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(DITHER16_FLAGS) $(DITHER16_SRCS) -o $@

strip4.elf: $(STRIP4_SRCS) $(LINKER_SCRIPT) $(STRIP4_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(STRIP4_FLAGS) $(STRIP4_SRCS) -o $@

strip5.elf: $(STRIP5_SRCS) $(LINKER_SCRIPT_072) $(STRIP5_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_072) $(STRIP5_FLAGS) $(STRIP5_SRCS) -o $@

relayball.elf: $(RELAYBALL_SRCS) $(LINKER_SCRIPT) $(RELAYBALL_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(RELAYBALL_FLAGS) $(RELAYBALL_SRCS) -o $@

uartball16.elf: $(UARTBALL16_SRCS) $(LINKER_SCRIPT) $(UARTBALL16_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(UARTBALL16_FLAGS) $(UARTBALL16_SRCS) -o $@

ws2801par.elf: $(WS2801PAR_SRCS) $(LINKER_SCRIPT_072) $(WS2801PAR_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_072) $(WS2801PAR_FLAGS) $(WS2801PAR_SRCS) -o $@

jitter_timer.elf: $(JITTER_TIMER_SRCS) $(LINKER_SCRIPT) $(JITTER_TIMER_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(JITTER_TIMER_FLAGS) $(JITTER_TIMER_SRCS) -o $@

timing_experiment.elf: $(TIMING_EXPERIMENT_SRCS) $(LINKER_SCRIPT) $(TIMING_EXPERIMENT_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TIMING_EXPERIMENT_FLAGS) $(TIMING_EXPERIMENT_SRCS) -o $@

