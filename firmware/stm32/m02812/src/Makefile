
GCC=arm-none-eabi-gcc
CFLAGS=-Wall -g -std=c99 -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb -ffunction-sections -fdata-sections -I platform -I. -Os
LDFLAGS=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT)
LDFLAGS_072=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT_072)

LINKER_SCRIPT=platform/stm32f030.ld
LINKER_SCRIPT_072=platform/stm32f072.ld
STARTUP_SRC=platform/startup_stm32f0xx.s

TARGETS=                  \
	main.bin              \
	dither16.bin          \
	edgeball16.bin        \
	relayball.bin         \
	uartball16.bin        \
	jitter_timer.bin      \
	timing_experiment.bin \
	strip5.bin

UARTBALL16_FLAGS=
EDGEBALL16_FLAGS=-DN_VALUES_PER_LED=3 -DN_LEDS_PER_STRIP=60
DITHER16_FLAGS=
MAIN_FLAGS=
RELAYBALL_FLAGS=
JITTER_TIMER_FLAGS=
TIMING_EXPERIMENT_FLAGS=
STRIP5_FLAGS=


EDGEBALL16_SRCS=edgeball16.c bitbang16_4strip.S util.c $(STARTUP_SRC)
UARTBALL16_SRCS=uartball16.c util.c $(STARTUP_SRC)
RELAYBALL_SRCS=relayball.S relayball_init.c relayball_bitbang.S util.c fsm.c $(STARTUP_SRC)
MAIN_SRCS=main.c bitbang.S util.c $(STARTUP_SRC)
DITHER16_SRCS=dither16.c dither16_bitbang.S util.c $(STARTUP_SRC)
JITTER_TIMER_SRCS=jitter_timer.c jitter_timer.S $(STARTUP_SRC)
TIMING_EXPERIMENT_SRCS=timing_experiment.c timing_experiment.S $(STARTUP_SRC)
STRIP5_SRCS=strip5.S strip5_init.c strip5_bitbang.S util.c fsm.c $(STARTUP_SRC)
HEADERS=dither16.h bitbang16_4strip.h bitbang.h util.h m0delay.S fsm.h uartball16.h jitter_timer.h

.PHONY: all clean test

.PRECIOUS: %.elf

all:$(TARGETS)

clean:
	-rm $(TARGETS) *.elf test/*.pyc

test: $(TARGETS)
	test/test_relay16.py relayball.elf
	test/test_bitbang.py main.elf 
	test/test_bitbang16.py dither16.elf
	test/test_edgeball16.py edgeball16.elf

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $< $@

dither16.elf: $(DITHER16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(DITHER16_FLAGS) $(DITHER16_SRCS) -o $@

edgeball16.elf: $(EDGEBALL16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(EDGEBALL16_FLAGS) $(EDGEBALL16_SRCS) -o $@

uartball16.elf: $(UARTBALL16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(UARTBALL16_FLAGS) $(UARTBALL16_SRCS) -o $@

relayball.elf: $(RELAYBALL_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(RELAYBALL_FLAGS) $(RELAYBALL_SRCS) -o $@

jitter_timer.elf: $(JITTER_TIMER_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(JITTER_TIMER_FLAGS) $(JITTER_TIMER_SRCS) -o $@

timing_experiment.elf: $(TIMING_EXPERIMENT_SRCS) $(LINKER_SCRIPT) $(HEADERS) timing_experiment.h
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TIMING_EXPERIMENT_FLAGS) $(TIMING_EXPERIMENT_SRCS) -o $@

strip5.elf: $(STRIP5_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_072) $(STRIP5_FLAGS) $(STRIP5_SRCS) -o $@

main.elf: $(MAIN_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(MAIN_FLAGS) $(MAIN_SRCS) -o $@

