# Name: Makefile
# Author: <insert your name here>
# Copyright: <insert your copyright message here>
# License: <insert your license reference here>

#DEVICE     = atmega1284p
DEVICE     = attiny45
CLOCK      = 8000000
PROGRAMMER = -c linuxspi -P /dev/spidev0.0
SRCDIR     = src
BIN        = bin
OBJECTS    = $(SRCDIR)/clockgen.o

FUSES      = -U lfuse:w:0x22:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m -U lock:w:0xFF:m

# For computing fuse byte values for other devices and options see
# the fuse bit calculator at http://www.engbedded.com/fusecalc/
# also http://eleccelerator.com/fusecalc/fusecalc.php

# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) -V
COMPILE = avr-g++ -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -std=gnu99 $(INCLUDE)
COMPILECPP = avr-g++ -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -std=c++11 $(INCLUDE)

PI_ADDRESS = raspberrypi.local

# pin definitions

RESET_PIN = 6

# symbolic targets:
all:	clockgen.hex

.c.o:
	$(COMPILE) -c $< -o $@

.cpp.o:
	$(COMPILECPP) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

%.flash:
	$(AVRDUDE) -U flash:w:$*.hex

%.flashbin:
	$(AVRDUDE) -U flash:w:$*.bin -V

fuse:
	$(AVRDUDE) -b115200 $(FUSES)

%.program: %.hex
	make progenable
	sudo make $*.install
	make progdisable

%.programbin:
	make progenable
	sudo make $*.installbin
	make progdisable

%.install: %.hex
	make fuse
	make $*.flash

%.installbin:
	make fuse
	make $*.flashbin

progenable:
	gpio mode 12 alt0
	gpio mode 13 alt0
	gpio mode 14 alt0
	gpio mode $(RESET_PIN) in

progdisable:
	gpio mode 12 in
	gpio mode 13 in
	gpio mode 14 in
	gpio mode $(RESET_PIN) out
	gpio write $(RESET_PIN) 0
	gpio write $(RESET_PIN) 1

clean:
	rm -f *.hex *.elf *.o *.bin
	rm -f $(SRCDIR)/*.o
	rm -f $(BIN)/*


# file targets:
%.elf: bindir $(OBJECTS)
	$(COMPILECPP) -o $*.elf $(OBJECTS)

%.hex: bindir %.elf
	rm -f $*.heximage
	avr-objcopy -j .text -j .data -O ihex $*.elf $*.hex
	avr-size --format=avr --mcu=$(DEVICE) $*.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flassh" target.

%.bin: bindir %.elf
	rm -f $*.bin
	avr-objcopy -j .text -j .data -O binary $*.elf $*.bin
	avr-size --format=avr --mcu=$(DEVICE) $*.elf

# Targets for code debugging and analysis:
disasm:	%.elf
	avr-objdump -d $*.elf

bindir:
	mkdir -p bin

reset:
	gpio write $(RESET_PIN) 0
	gpio write $(RESET_PIN) 1


