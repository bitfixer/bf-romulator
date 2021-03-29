clean:
		rm -f *.o
		rm -f *.out

%.o: %.s %.cfg
	../cc65/bin/ca65 $< -o $@

%.out: %.o %.cfg
	../cc65/bin/ld65 -o $@ -C $*.cfg $<

%.bin: %.o %.cfg
	../cc65/bin/ld65 -o $@ -C $*.cfg $<

ramtest.bin: ramtest.c ramtest.cfg preinit.s
	../cc65/bin/cc65 ramtest.c -Oi --inline-stdfuncs -o ramtest.s
	../cc65/bin/ca65 ramtest.s -o ramtest.o
	../cc65/bin/ca65 preinit.s -o preinit.o
	../cc65/bin/ld65 -o ramtest.bin ramtest.o preinit.o --lib none.lib -C ramtest.cfg -m ramtest.map -vm

nop: nop.c
	gcc nop.c -o nop

nop.bin: nop
	./nop > nop.bin