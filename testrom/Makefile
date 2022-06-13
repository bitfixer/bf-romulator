clean:
		rm -f *.o
		rm -f *.out
		rm -f *.bin

%.o: %.s %.cfg
	../cc65/bin/ca65 $< -o $@

%.out: %.o %.cfg
	../cc65/bin/ld65 -o $@ -C $*.cfg $<

%.bin: %.o %.cfg
	../cc65/bin/ld65 -o $@ -C $*.cfg $<

ramtest.bin: ramtest.c ramtest.cfg preinit.s
	../cc65/bin/cc65 ramtest.c -I ../cc65/include -Oi --inline-stdfuncs -o ramtest.s
	../cc65/bin/ca65 ramtest.s -I ../cc65/asminc -o ramtest.o
	../cc65/bin/ca65 preinit.s -I ../cc65/asminc -o preinit.o
	../cc65/bin/ld65 -L ../cc65/lib -o ramtest.bin ramtest.o preinit.o --lib none.lib -C ramtest.cfg -m ramtest.map -vm

boot_standalone.bin: boot_standalone.c boot_standalone.cfg preinit.s
	../cc65/bin/cc65 boot_standalone.c -I ../cc65/include -Oi --inline-stdfuncs -o boot_standalone.s
	../cc65/bin/ca65 boot_standalone.s -I ../cc65/asminc -o boot_standalone.o
	../cc65/bin/ca65 preinit.s -I ../cc65/asminc -o preinit.o
	../cc65/bin/ld65 -L ../cc65/lib -o boot_standalone.bin boot_standalone.o preinit.o --lib none.lib -C boot_standalone.cfg -m boot_standalone.map -vm

ieee_test.bin: ieee_test.c ieee_test.cfg preinit.s
	../cc65/bin/cc65 ieee_test.c -I ../cc65/include -Oi --inline-stdfuncs -o ieee_test.s
	../cc65/bin/ca65 ieee_test.s -I ../cc65/asminc -o ieee_test.o
	../cc65/bin/ca65 preinit.s -I ../cc65/asminc -o preinit.o
	../cc65/bin/ld65 -L ../cc65/lib -o ieee_test.bin ieee_test.o preinit.o --lib none.lib -C ieee_test.cfg -m ieee_test.map -vm

nop: nop.c
	gcc nop.c -o nop

nop.bin: nop
	./nop > nop.bin