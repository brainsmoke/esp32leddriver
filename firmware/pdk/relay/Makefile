
TARGETS=relay_oldpcb-pdk13.ihx relay_newpcb-pdk13.ihx relay5-pdk13.ihx relay_oldpcb-pdk14.ihx relay_newpcb-pdk14.ihx relay5-pdk14.ihx
CLEAN=$(TARGETS)

AS13=sdaspdk13
AS14=sdaspdk14

all: $(TARGETS)

clean:
	-rm $(CLEAN)

%-pdk13.rel: %.asm pdk.asm
	$(AS13) -s -o -l $@ $<

%-pdk14.rel: %.asm pdk.asm
	$(AS14) -s -o -l $@ $<

%.ihx: %.rel
	sdldpdk -muwx -i $@ -Y $< -e
	-rm $(@:.ihx=.cdb) $(@:.ihx=.lst) $(@:.ihx=.map) $(@:.ihx=.rel) $(@:.ihx=.rst) $(@:.ihx=.sym)
