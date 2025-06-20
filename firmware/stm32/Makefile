
GCC=arm-none-eabi-gcc
CFLAGS=-Wall -g -std=c99 -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb -ffunction-sections -fdata-sections -I platform -I common -I. -Os
CFLAGS_GPIOB=$(CFLAGS) -DUSE_GPIOB=1
LDFLAGS=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT)
LDFLAGS_072=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT_072)
LDFLAGS_030CC=-Wl,--gc-sections -Lplatform -T../$(LINKER_SCRIPT_030CC)

LINKER_SCRIPT=platform/stm32f030.ld
LINKER_SCRIPT_072=platform/stm32f072.ld
LINKER_SCRIPT_030CC=platform/stm32f030cc.ld
STARTUP_SRC=platform/startup_stm32f0xx.s

WS2812_DMA16_ELF=ws2812_dma16/ws2812_dma16.elf
WS2812_DMA16_HEADERS=ws2812_dma16/config.h ws2812_dma16/ws2812_dma16.h common/ws2812_common.h common/ws2812_dma.h common/util.h common/m0delay.S
WS2812_DMA16_SRCS=ws2812_dma16/asm_routines.S ws2812_dma16/main.c common/ws2812_dma.c common/util.c $(STARTUP_SRC)
WS2812_DMA16_FLAGS=-I ws2812_dma16


PERFECTION_ELF=perfection/perfection.elf
PERFECTION_HEADERS=perfection/config.h perfection/ws2812_dma16.h common/ws2812_common.h common/ws2812_dma.h common/util.h common/m0delay.S common/fsm.h
PERFECTION_SRCS=perfection/asm_routines.S perfection/main.c common/ws2812_dma.c common/util.c common/fsm.c $(STARTUP_SRC)
PERFECTION_FLAGS=-I perfection

UARTRELAY_ELF=uartrelay/uartrelay.elf
UARTRELAY_HEADERS=uartrelay/config.h uartrelay/uartrelay.h common/ws2812_common.h common/ws2812_dma.h common/util.h common/m0delay.S common/fsm.h
UARTRELAY_SRCS=uartrelay/main.c common/ws2812_dma.c common/util.c common/fsm.c $(STARTUP_SRC)
UARTRELAY_FLAGS=-I uartrelay

UARTDEMUX_ELF=uartdemux/uartdemux.elf
UARTDEMUX_HEADERS=uartdemux/config.h common/gpio_dma.h common/util.h common/m0delay.S common/fsm.h
UARTDEMUX_SRCS=uartdemux/main.c common/gpio_dma.c common/util.c common/fsm.c $(STARTUP_SRC)
UARTDEMUX_FLAGS=-I uartdemux

WS2801PAR_ELF=ws2801par/ws2801par.elf
WS2801PAR_HEADERS=ws2801par/ws2801par.h common/util.h common/m0delay.S
WS2801PAR_SRCS=ws2801par/ws2801par.c ws2801par/ws2801par_bitbang.S common/util.c $(STARTUP_SRC)
WS2801PAR_FLAGS=-I ws2801par

TIMING_EXPERIMENT_ELF=timing_experiment/timing_experiment.elf
TIMING_EXPERIMENT_HEADERS=timing_experiment/timing_experiment.h common/m0delay.S
TIMING_EXPERIMENT_SRCS=timing_experiment/timing_experiment.c timing_experiment/timing_experiment.S $(STARTUP_SRC)
TIMING_EXPERIMENT_FLAGS=-I timing_experiment

DITHER8_ELF=dither8/dither8.elf
DITHER8_HEADERS=dither8/dither8.h common/util.h common/m0delay.S
DITHER8_SRCS=dither8/dither8.c dither8/dither8_bitbang.S common/util.c $(STARTUP_SRC)
DITHER8_FLAGS=-I dither8

DITHER16_ELF=dither16/dither16.elf
DITHER16_HEADERS=dither16/dither16.h common/util.h common/m0delay.S
DITHER16_SRCS=dither16/dither16.c dither16/dither16_bitbang.S common/util.c $(STARTUP_SRC)
DITHER16_FLAGS=-I dither16

STRIP5_ELF=strip5/strip5.elf
STRIP5_HEADERS=strip5/strip5.h common/util.h common/m0delay.S
STRIP5_SRCS=strip5/strip5.S strip5/strip5_init.c strip5/strip5_bitbang.S common/util.c common/fsm.c $(STARTUP_SRC)
STRIP5_FLAGS=-I strip5

STRIP5_GPIOB_ELF=strip5/strip5_gpiob.elf

STRIP4_ELF=strip4/strip4.elf
STRIP4_HEADERS=strip4/strip4.h common/util.h common/fsm.h common/m0delay.S
STRIP4_SRCS=strip4/strip4.c strip4/strip4_bitbang.S common/util.c $(STARTUP_SRC)
STRIP4_FLAGS=-I strip4

RELAYBALL_ELF=relayball/relayball.elf
RELAYBALL_HEADERS=relayball/relayball.h common/util.h common/fsm.h common/m0delay.S
RELAYBALL_SRCS=relayball/relayball.S relayball/relayball_init.c relayball/relayball_bitbang.S common/util.c common/fsm.c $(STARTUP_SRC)
RELAYBALL_FLAGS=-I relayball

UARTBALL16_ELF=uartball16/uartball16.elf
UARTBALL16_HEADERS=uartball16/uartball16.h common/util.h
UARTBALL16_SRCS=uartball16/uartball16.c common/util.c $(STARTUP_SRC)
UARTBALL16_FLAGS=-I uartball16

OBEGRAENSAD_ELF=obegraensad/obegraensad.elf
OBEGRAENSAD_HEADERS=obegraensad/obegraensad.h common/util.h common/m0delay.S
OBEGRAENSAD_SRCS=obegraensad/obegraensad.c obegraensad/bitbang.S common/util.c $(STARTUP_SRC)
OBEGRAENSAD_FLAGS=-I obegraensad

JITTER_TIMER_ELF=jitter_timer/jitter_timer.elf
JITTER_TIMER_HEADERS=jitter_timer/jitter_timer.h common/m0delay.S
JITTER_TIMER_SRCS=jitter_timer/jitter_timer.c jitter_timer/jitter_timer.S $(STARTUP_SRC)
JITTER_TIMER_FLAGS=-I jitter_timer

M0MATRIX_ELF=m0matrix/m0matrix.elf
M0MATRIX_HEADERS=m0matrix/bitbang.h m0matrix/util.h common/m0delay.S
M0MATRIX_SRCS=m0matrix/main.c m0matrix/bitbang.S m0matrix/util.c $(STARTUP_SRC)
M0MATRIX_FLAGS=-I m0matrix

M0MATRIX_8_TO_16_ELF=m0matrix_8_to_16/m0matrix_8_to_16.elf
M0MATRIX_8_TO_16_HEADERS=m0matrix_8_to_16/bitbang.h m0matrix_8_to_16/util.h common/m0delay.S
M0MATRIX_8_TO_16_SRCS=m0matrix_8_to_16/main.c m0matrix_8_to_16/bitbang.S m0matrix_8_to_16/util.c $(STARTUP_SRC)
M0MATRIX_8_TO_16_FLAGS=-I m0matrix_8_to_16

ELFS=                        \
	$(DITHER8_ELF)           \
	$(DITHER16_ELF)          \
	$(STRIP4_ELF)            \
	$(STRIP5_ELF)            \
	$(STRIP5_GPIOB_ELF)      \
	$(RELAYBALL_ELF)         \
	$(UARTBALL16_ELF)        \
	$(WS2801PAR_ELF)         \
	$(WS2812_DMA16_ELF)      \
	$(UARTRELAY_ELF)         \
	$(UARTDEMUX_ELF)         \
	$(OBEGRAENSAD_ELF)       \
	$(JITTER_TIMER_ELF)      \
	$(TIMING_EXPERIMENT_ELF) \
	$(PERFECTION_ELF)        \
    $(M0MATRIX_ELF)          \
    $(M0MATRIX_8_TO_16_ELF)

TARGETS=$(patsubst %.elf, %.bin, $(ELFS))

.PHONY: all clean test

.PRECIOUS: %.elf

all:$(TARGETS)

clean:
	-rm $(TARGETS) $(ELFS) test/*.pyc

test: $(TARGETS)
	test/test_bitbang.py $(DITHER8_ELF)
	test/test_bitbang16.py $(DITHER16_ELF)
	test/test_strip4.py $(STRIP4_ELF)
	test/test_strip5.py $(STRIP5_ELF)
	test/test_relay16.py $(RELAYBALL_ELF)

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $< $@

$(DITHER8_ELF): $(DITHER8_SRCS) $(LINKER_SCRIPT) $(DITHER8_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(DITHER8_FLAGS) $(DITHER8_SRCS) -o $@

$(DITHER16_ELF): $(DITHER16_SRCS) $(LINKER_SCRIPT) $(DITHER16_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(DITHER16_FLAGS) $(DITHER16_SRCS) -o $@

$(STRIP4_ELF): $(STRIP4_SRCS) $(LINKER_SCRIPT) $(STRIP4_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(STRIP4_FLAGS) $(STRIP4_SRCS) -o $@

$(STRIP5_ELF): $(STRIP5_SRCS) $(LINKER_SCRIPT_072) $(STRIP5_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_072) $(STRIP5_FLAGS) $(STRIP5_SRCS) -o $@

$(STRIP5_GPIOB_ELF): $(STRIP5_SRCS) $(LINKER_SCRIPT_072) $(STRIP5_HEADERS)
	$(GCC) $(CFLAGS_GPIOB) $(LDFLAGS_072) $(STRIP5_FLAGS) $(STRIP5_SRCS) -o $@

$(RELAYBALL_ELF): $(RELAYBALL_SRCS) $(LINKER_SCRIPT) $(RELAYBALL_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(RELAYBALL_FLAGS) $(RELAYBALL_SRCS) -o $@

$(UARTBALL16_ELF): $(UARTBALL16_SRCS) $(LINKER_SCRIPT) $(UARTBALL16_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(UARTBALL16_FLAGS) $(UARTBALL16_SRCS) -o $@

$(WS2801PAR_ELF): $(WS2801PAR_SRCS) $(LINKER_SCRIPT_072) $(WS2801PAR_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_072) $(WS2801PAR_FLAGS) $(WS2801PAR_SRCS) -o $@

$(WS2812_DMA16_ELF): $(WS2812_DMA16_SRCS) $(LINKER_SCRIPT_030CC) $(WS2812_DMA16_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_030CC) $(WS2812_DMA16_FLAGS) $(WS2812_DMA16_SRCS) -o $@

$(PERFECTION_ELF): $(PERFECTION_SRCS) $(LINKER_SCRIPT_030CC) $(PERFECTION_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_030CC) $(PERFECTION_FLAGS) $(PERFECTION_SRCS) -o $@

$(UARTRELAY_ELF): $(UARTRELAY_SRCS) $(LINKER_SCRIPT) $(UARTRELAY_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(UARTRELAY_FLAGS) $(UARTRELAY_SRCS) -o $@

$(UARTDEMUX_ELF): $(UARTDEMUX_SRCS) $(LINKER_SCRIPT_030CC) $(UARTDEMUX_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS_030CC) $(UARTDEMUX_FLAGS) $(UARTDEMUX_SRCS) -o $@

$(OBEGRAENSAD_ELF): $(OBEGRAENSAD_SRCS) $(LINKER_SCRIPT) $(OBEGRAENSAD_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(OBEGRAENSAD_FLAGS) $(OBEGRAENSAD_SRCS) -o $@

$(JITTER_TIMER_ELF): $(JITTER_TIMER_SRCS) $(LINKER_SCRIPT) $(JITTER_TIMER_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(JITTER_TIMER_FLAGS) $(JITTER_TIMER_SRCS) -o $@

$(TIMING_EXPERIMENT_ELF): $(TIMING_EXPERIMENT_SRCS) $(LINKER_SCRIPT) $(TIMING_EXPERIMENT_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(TIMING_EXPERIMENT_FLAGS) $(TIMING_EXPERIMENT_SRCS) -o $@

$(M0MATRIX_ELF): $(M0MATRIX_SRCS) $(LINKER_SCRIPT) $(M0MATRIX_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(M0MATRIX_FLAGS) $(M0MATRIX_SRCS) -o $@

$(M0MATRIX_8_TO_16_ELF): $(M0MATRIX_8_TO_16_SRCS) $(LINKER_SCRIPT) $(M0MATRIX_8_TO_16_HEADERS)
	$(GCC) $(CFLAGS) $(LDFLAGS) $(M0MATRIX_8_TO_16_FLAGS) $(M0MATRIX_8_TO_16_SRCS) -o $@

