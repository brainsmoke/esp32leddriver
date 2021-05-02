
GCC=arm-none-eabi-gcc
CFLAGS=-Wall -g -std=c99 -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb -ffunction-sections -fdata-sections -I platform -I. -Os
LDFLAGS=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT)

LINKER_SCRIPT=platform/stm32f030.ld
STARTUP_SRC=platform/startup_stm32f0xx.s

TARGETS=main.elf main.bin main16.elf main16.bin edgeball16.elf edgeball16.bin relayball.elf relayball.bin
TESTS=test/test.elf test/test16.elf test/test_edgeball16.elf test/test_relay16.elf test/test_edgeball16alt.elf

EDGEBALL16_FLAGS=-DN_VALUES_PER_LED=3 -DN_LEDS_PER_STRIP=60
MAIN16_FLAGS=-DN_VALUES_PER_LED=3 -DN_LEDS_PER_STRIP=15
MAIN_FLAGS=
TEST16_FLAGS=-DN_VALUES_PER_LED=3 -DN_LEDS_PER_STRIP=15
TEST_RELAY16_FLAGS=
TEST_EDGEBALL16_FLAGS=
TEST_EDGEBALL16ALT_FLAGS=
TEST_FLAGS=
RELAYBALL_FLAGS=


EDGEBALL16_SRCS=edgeball16.c bitbang16_4strip.S util.c $(STARTUP_SRC)
RELAYBALL_SRCS=relayball.S relayball_init.c default_route.c bitbang16_relay_4strip.S util.c fsm.c $(STARTUP_SRC)
MAIN_SRCS=main.c bitbang.S util.c $(STARTUP_SRC)
MAIN16_SRCS=main16.c bitbang16.S util.c $(STARTUP_SRC)
TEST_SRCS=test/test.c bitbang.S util.c $(STARTUP_SRC)
TEST16_SRCS=test/test16.c bitbang16.S util.c $(STARTUP_SRC)
TEST_EDGEBALL16_SRCS=test/test_edgeball16.c bitbang16_4strip.S util.c $(STARTUP_SRC)
TEST_EDGEBALL16ALT_SRCS=test/test_edgeball16.c bitbang16_alt_4strip.S util.c $(STARTUP_SRC)
TEST_RELAY16_SRCS=test/test_relay16.c bitbang16_relay_4strip.S util.c $(STARTUP_SRC)
HEADERS=bitbang16.h bitbang16_4strip.h bitbang16_relay_4strip.h bitbang.h util.h m0delay.S fsm.h

.PHONY: all clean test

all:$(TARGETS)

clean:
	-rm $(TARGETS) $(TESTS) test/*.pyc

test: $(TESTS)
	test/test_relay16.py test/test_relay16.elf
	test/test_edgeball16.py test/test_edgeball16alt.elf
	test/test_bitbang.py test/test.elf 
	test/test_bitbang16.py test/test16.elf
	test/test_edgeball16.py test/test_edgeball16.elf

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $< $@

main16.elf: $(MAIN16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(MAIN16_FLAGS) $(MAIN16_SRCS) -o $@

edgeball16.elf: $(EDGEBALL16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(EDGEBALL16_FLAGS) $(EDGEBALL16_SRCS) -o $@

relayball.elf: $(RELAYBALL_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(RELAYBALL_FLAGS) $(RELAYBALL_SRCS) -o $@

main.elf: $(MAIN_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(MAIN_FLAGS) $(MAIN_SRCS) -o $@

test/test16.elf: $(TEST16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TEST16_FLAGS) $(TEST16_SRCS) -o $@

test/test_edgeball16.elf: $(TEST_EDGEBALL16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TEST16_FLAGS) $(TEST_EDGEBALL16_SRCS) -o $@

test/test_relay16.elf: $(TEST_RELAY16_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TEST_RELAY16_FLAGS) $(TEST_RELAY16_SRCS) -o $@

test/test_edgeball16alt.elf: $(TEST_EDGEBALL16ALT_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TEST_EDGEBALL16ALT_FLAGS) $(TEST_EDGEBALL16ALT_SRCS) -o $@

test/test.elf: $(TEST_SRCS) $(LINKER_SCRIPT) $(HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TEST_FLAGS) $(TEST_SRCS) -o $@

