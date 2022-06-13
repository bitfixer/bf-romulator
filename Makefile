# ROMulator - RAM/ROM replacement and diagnostic for 8-bit CPUs
# Copyright (C) 2019  Michael Hill

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Makefile to build ROMulator targets

PROGRAMMER_DIR := programmer
TOOLS_DIR := tools
CONFIG_DIR := config
ROMULATOR_DIR := romulator
ROMULATOR_Z80_DIR := romulator_z80
ROMS_DIR := roms
SHARED_DIR := bf-shared
CONFIG := default
WEBSERVER_DIR := webserver
MEMORY_SET := $(shell pwd)/$(CONFIG_DIR)/memory_set_$(CONFIG).csv
ENABLE_TABLE := $(shell pwd)/$(CONFIG_DIR)/enable_table_$(CONFIG).csv
MEMORY_SET_Z80 := $(shell pwd)/$(CONFIG_DIR)/memory_set_$(CONFIG)_z80.csv
ENABLE_TABLE_Z80 := $(shell pwd)/$(CONFIG_DIR)/enable_table_$(CONFIG)_z80.csv
BIN_DIR := bin
REMOTE := raspberrypi.local

#pin definitions
DBG := 27
RST := 6
CS := 10

# Programmer

$(BIN_DIR)/programmer: $(PROGRAMMER_DIR)/programmer.cc
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/programmer $(PROGRAMMER_DIR)/programmer.cc -lwiringPi

$(BIN_DIR)/programmer_spi: $(PROGRAMMER_DIR)/programmer_spi.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/programmer_spi $(PROGRAMMER_DIR)/programmer_spi.cpp -lwiringPi

$(BIN_DIR)/make_screen_image: $(TOOLS_DIR)/make_screen_image.cpp $(TOOLS_DIR)/libRomulatorVram.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/make_screen_image $(TOOLS_DIR)/make_screen_image.cpp $(TOOLS_DIR)/libRomulatorVram.cpp

$(BIN_DIR)/make_test_vram: $(PROGRAMMER_DIR)/make_test_vram.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/make_test_vram $(PROGRAMMER_DIR)/make_test_vram.cpp

# Tools

$(BIN_DIR)/build_memory_map_set: $(TOOLS_DIR)/build_memory_map_set.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/build_memory_map_set $(TOOLS_DIR)/build_memory_map_set.cpp

$(BIN_DIR)/build_enable_table: $(TOOLS_DIR)/build_enable_table.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/build_enable_table $(TOOLS_DIR)/build_enable_table.cpp

$(BIN_DIR)/makerom: $(TOOLS_DIR)/makerom.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/makerom $(TOOLS_DIR)/makerom.cpp

$(BIN_DIR)/verify_map_setting: $(TOOLS_DIR)/verify_map_setting.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/verify_map_setting $(TOOLS_DIR)/verify_map_setting.cpp

$(BIN_DIR)/crc32: $(PROGRAMMER_DIR)/crc32.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/crc32 $(PROGRAMMER_DIR)/crc32.cpp

$(BIN_DIR)/console: $(TOOLS_DIR)/console.cpp $(TOOLS_DIR)/libRomulatorDebug.h $(TOOLS_DIR)/libRomulatorDebug.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/console $(TOOLS_DIR)/console.cpp $(TOOLS_DIR)/libRomulatorDebug.cpp -lwiringPi

$(BIN_DIR)/fetch_roms: $(TOOLS_DIR)/fetch_roms.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/fetch_roms $(TOOLS_DIR)/fetch_roms.cpp

fetch_roms: $(BIN_DIR)/fetch_roms $(MEMORY_SET)
	mkdir -p $(ROMS_DIR)
	cd $(ROMS_DIR); ../$(BIN_DIR)/fetch_roms $(MEMORY_SET) $(BASEURL)

$(BIN_DIR)/webserver: $(TOOLS_DIR)/webserver.cpp $(TOOLS_DIR)/libbmp.h $(TOOLS_DIR)/libbmp.cpp $(TOOLS_DIR)/libRomulatorVram.cpp $(TOOLS_DIR)/libRomulatorDebug.cpp $(SHARED_DIR)/timer.cpp
	g++ -o $(BIN_DIR)/webserver $(TOOLS_DIR)/webserver.cpp $(TOOLS_DIR)/libbmp.cpp $(TOOLS_DIR)/libRomulatorVram.cpp $(TOOLS_DIR)/libRomulatorDebug.cpp $(SHARED_DIR)/timer.cpp -lwiringPi -lpng -lpthread

$(BIN_DIR)/webserver_test: $(TOOLS_DIR)/webserver.cpp $(TOOLS_DIR)/libbmp.h $(TOOLS_DIR)/libbmp.cpp $(TOOLS_DIR)/libRomulatorVram.cpp $(SHARED_DIR)/timer.cpp
	g++ -DTEST=1 -o $(BIN_DIR)/webserver_test $(TOOLS_DIR)/webserver.cpp $(TOOLS_DIR)/libbmp.cpp $(TOOLS_DIR)/libRomulatorVram.cpp $(SHARED_DIR)/timer.cpp -lpng

.PHONY: webserver
webserver: $(BIN_DIR)/webserver
	bin/webserver -r $(WEBSERVER_DIR)

# FPGA

$(BIN_DIR)/memorymap.bin: $(MEMORY_SET) $(BIN_DIR)/build_memory_map_set $(BIN_DIR)/random_test.bin $(BIN_DIR)/testrom.out $(BIN_DIR)/testrom_appleii.out $(BIN_DIR)/testromv2.out $(BIN_DIR)/ramtest.bin $(BIN_DIR)/boot_standalone.bin $(BIN_DIR)/nop.bin $(BIN_DIR)/ieee_test.bin
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/build_memory_map_set -d $(ROMS_DIR)/ < $(MEMORY_SET) > $(BIN_DIR)/memorymap.bin

$(BIN_DIR)/memorymap_z80.bin: $(MEMORY_SET_Z80) $(BIN_DIR)/build_memory_map_set
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/build_memory_map_set -d $(ROMS_DIR)/ < $(MEMORY_SET_Z80) > $(BIN_DIR)/memorymap_z80.bin

$(BIN_DIR)/enable_table.bin: $(BIN_DIR)/build_enable_table $(ENABLE_TABLE)
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/build_enable_table $(ENABLE_TABLE) $(BIN_DIR)/enable_table.bin > $(BIN_DIR)/enable_table.txt

$(BIN_DIR)/enable_table_z80.bin: $(BIN_DIR)/build_enable_table $(ENABLE_TABLE_Z80)
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/build_enable_table $(ENABLE_TABLE_Z80) $(BIN_DIR)/enable_table_z80.bin > $(BIN_DIR)/enable_table_z80.txt

$(BIN_DIR)/crc32_table.txt: $(BIN_DIR)/crc32
	$(BIN_DIR)/crc32 -t -x > $(BIN_DIR)/crc32_table.txt

$(BIN_DIR)/hardware_6502.bin: $(ROMULATOR_DIR)/*.v $(ROMULATOR_DIR)/6502/*.v $(BIN_DIR)/enable_table.bin $(BIN_DIR)/crc32_table.txt $(BIN_DIR)/vram_test.txt
	mkdir -p $(BIN_DIR)
	cd $(ROMULATOR_DIR); rm -f input*.v; rm -f *.pcf
	cp $(ROMULATOR_DIR)/6502/* $(ROMULATOR_DIR)
	cd $(ROMULATOR_DIR); rm -f hardware.*; apio build
	cp $(ROMULATOR_DIR)/hardware.bin $(BIN_DIR)/hardware_6502.bin
	rm $(ROMULATOR_DIR)/hardware.*
	rm $(ROMULATOR_DIR)/input6502.v; rm $(ROMULATOR_DIR)/up5k.pcf

$(BIN_DIR)/hardware_z80.bin: $(ROMULATOR_DIR)/*.v $(ROMULATOR_DIR)/z80/*.v $(BIN_DIR)/enable_table_z80.bin $(BIN_DIR)/crc32_table.txt $(BIN_DIR)/vram_test.txt
	mkdir -p $(BIN_DIR)
	cd $(ROMULATOR_DIR); rm -f input*.v; rm -f *.pcf
	cp $(ROMULATOR_DIR)/z80/* $(ROMULATOR_DIR)
	cd $(ROMULATOR_DIR); rm -f hardware*.*; apio build
	cp $(ROMULATOR_DIR)/hardware.bin $(BIN_DIR)/hardware_z80.bin
	rm $(ROMULATOR_DIR)/hardware.*
	rm $(ROMULATOR_DIR)/inputZ80.v; rm $(ROMULATOR_DIR)/up5k.pcf

.PHONY: romulator
romulator: $(BIN_DIR)/romulator.bin

.PHONY: romulator_z80
romulator_z80: $(BIN_DIR)/romulator_z80.bin

$(BIN_DIR)/romulator.bin: $(BIN_DIR)/makerom $(BIN_DIR)/hardware_6502.bin $(BIN_DIR)/memorymap.bin $(BIN_DIR)/enable_table.bin
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/makerom $(BIN_DIR)/hardware_6502.bin $(BIN_DIR)/memorymap.bin $(BIN_DIR)/enable_table.bin > $(BIN_DIR)/romulator.bin

$(BIN_DIR)/romulator_z80.bin: $(BIN_DIR)/makerom $(BIN_DIR)/hardware_z80.bin $(BIN_DIR)/memorymap_z80.bin $(BIN_DIR)/enable_table_z80.bin
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/makerom $(BIN_DIR)/hardware_z80.bin $(BIN_DIR)/memorymap_z80.bin $(BIN_DIR)/enable_table_z80.bin > $(BIN_DIR)/romulator_z80.bin

# General

.PHONY: program
program_old: $(BIN_DIR)/romulator.bin $(BIN_DIR)/programmer
	$(BIN_DIR)/programmer -f < $(BIN_DIR)/romulator.bin

# initialize pigpio 
# set logic high on debug pin so romulator can start
.PHONY: init
init:
	gpio mode $(DBG) out
	gpio write $(DBG) 1
	gpio mode $(RST) out
	gpio mode $(CS) out

# enter debug mode
# set the chip select line high to allow romulator to start
# set the reset line to an input to prevent holding romulator in reset
.PHONY: debug
debug:
	gpio mode $(DBG) out
	gpio write $(DBG) 1
	gpio mode $(RST) in
	gpio mode $(CS) in

program: init reset $(BIN_DIR)/romulator.bin $(BIN_DIR)/programmer_spi
	$(BIN_DIR)/programmer_spi < $(BIN_DIR)/romulator.bin

readback: init $(BIN_DIR)/romulator.bin $(BIN_DIR)/programmer_spi
	$(BIN_DIR)/programmer_spi -r $(shell stat --printf="%s" $(BIN_DIR)/romulator.bin) > readback.bin
	diff readback.bin $(BIN_DIR)/romulator.bin

transfer: clean $(BIN_DIR)/romulator.bin
	scp $(BIN_DIR)/romulator.bin pi@$(REMOTE):~/bf-romulator/$(BIN_DIR)

# testing
$(BIN_DIR)/random_test.bin:
	dd if=/dev/urandom of=$(BIN_DIR)/random_test.bin bs=1 count=65536

$(BIN_DIR)/random_test.txt: $(BIN_DIR)/random_test.bin
	xxd $(BIN_DIR)/random_test.bin > $(BIN_DIR)/random_test.txt

console_test: $(BIN_DIR)/console $(BIN_DIR)/random_test.txt $(BIN_DIR)/crc32
	$(BIN_DIR)/crc32 < $(BIN_DIR)/random_test.bin
	$(BIN_DIR)/console -r > $(BIN_DIR)/console_readback.bin
	xxd $(BIN_DIR)/console_readback.bin > $(BIN_DIR)/console_readback.txt
	diff $(BIN_DIR)/console_readback.txt $(BIN_DIR)/random_test.txt

$(BIN_DIR)/testrom_sim: $(TOOLS_DIR)/testrom_sim.c $(TOOLS_DIR)/fake6502.c
	mkdir -p $(BIN_DIR)
	gcc -o $(BIN_DIR)/testrom_sim $(TOOLS_DIR)/testrom_sim.c $(TOOLS_DIR)/fake6502.c


$(BIN_DIR)/testrom.out: testrom/testrom.s testrom/testrom.cfg
	cd testrom; make testrom.out; cp testrom.out ../$(BIN_DIR)/testrom.out; rm testrom.out

$(BIN_DIR)/testromv2.out: testrom/testromv2.s testrom/testromv2.cfg
	cd testrom; make testromv2.out; cp testromv2.out ../$(BIN_DIR)/testromv2.out; rm testromv2.out

$(BIN_DIR)/testrom_appleii.out: testrom/testrom_appleii.s testrom/testrom_appleii.cfg
	cd testrom; make testrom_appleii.out; cp testrom_appleii.out ../$(BIN_DIR)/testrom_appleii.out; rm testrom_appleii.out

$(BIN_DIR)/ramtest.bin: testrom/ramtest.c testrom/preinit.s testrom/ramtest.cfg
	cd testrom; make ramtest.bin; cp ramtest.bin ../$(BIN_DIR)/ramtest.bin; rm ramtest.bin

$(BIN_DIR)/boot_standalone.bin: testrom/boot_standalone.c testrom/preinit.s testrom/boot_standalone.cfg
	cd testrom; make boot_standalone.bin; cp boot_standalone.bin ../$(BIN_DIR)/boot_standalone.bin; rm boot_standalone.bin

$(BIN_DIR)/ieee_test.bin: testrom/ieee_test.c testrom/preinit.s testrom/ieee_test.cfg
	cd testrom; make ieee_test.bin; cp ieee_test.bin ../$(BIN_DIR)/ieee_test.bin; rm ieee_test.bin

$(BIN_DIR)/nop.bin: testrom/nop.c
	cd testrom; make nop.bin; cp nop.bin ../$(BIN_DIR)/nop.bin; rm nop.bin

$(BIN_DIR)/vram_test.txt: $(BIN_DIR)/make_test_vram
	$(BIN_DIR)/make_test_vram > $(BIN_DIR)/vram_test.txt

$(BIN_DIR)/test_pet: $(TOOLS_DIR)/test_pet.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/test_pet -lwiringPi $(TOOLS_DIR)/test_pet.cpp

$(BIN_DIR)/test_appleii: $(TOOLS_DIR)/test_appleii.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/test_appleii -lwiringPi $(TOOLS_DIR)/test_appleii.cpp

.PHONY: reset
reset: $(BIN_DIR)/programmer
	$(BIN_DIR)/programmer -b

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(WEBSERVER_DIR)/webserver