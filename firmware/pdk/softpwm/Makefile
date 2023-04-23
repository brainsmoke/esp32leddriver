
TARGET13=leddriver16bit-pdk13.ihx
TARGET14=leddriver16bit-pdk14.ihx

TESTS=lowbits16_test-pdk14.ihx

TARGETS=$(TARGET13) $(TARGET14) $(TESTS)

CLEAN=$(TARGETS)
DEPS=pdk.asm uart2.asm delay.asm softpwm16.asm settings.asm pdk13/parts.asm pdk14/parts.asm

DEVICE=/dev/ttyACM0

AS13=sdaspdk13 -Ipdk13
AS14=sdaspdk14 -Ipdk14

PROG=easypdkprog

NAME13=PMS150C
NAME14=PFS154

ADDRESS=0

.PHONY: write13 erase13 write14 erase14

all: $(TARGETS)

clean:
	-rm $(CLEAN)
	-rm *.ihx *.cdb *.lst *.map *.rel *.rst *.sym

%-pdk13.rel: %.asm $(DEPS)
	$(AS13) -s -o -l $@ $<

%-pdk14.rel: %.asm $(DEPS)
	$(AS14) -s -o -l $@ $<

%.ihx: %.rel
	sdldpdk -muwx -g INDEX=$$(($(ADDRESS)*3)) -i $@ -Y $< -e
	-rm $(@:.ihx=.cdb) $(@:.ihx=.lst) $(@:.ihx=.map) $(@:.ihx=.rel) $(@:.ihx=.rst) $(@:.ihx=.sym)

write13: $(TARGET13)
	$(PROG) -p $(DEVICE) -n $(NAME13) write $(TARGET13)

erase13:
	$(PROG) -p $(DEVICE) -n $(NAME13) erase

write14: $(TARGET14)
	$(PROG) -p $(DEVICE) -n $(NAME14) write $(TARGET14)

erase14:
	$(PROG) -p $(DEVICE) -n $(NAME14) erase
