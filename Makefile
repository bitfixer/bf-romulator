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
ROMULATOR_DIR := romulator
ROMS_DIR := roms
CONFIG := default
MEMORY_SET := $(shell pwd)/$(TOOLS_DIR)/memory_set_$(CONFIG).csv
ENABLE_TABLE := $(shell pwd)/$(TOOLS_DIR)/enable_table_$(CONFIG).csv
BIN_DIR := bin

#pin definitions
DBG := 27
RST := 6
CS := 10

# Programmer

$(BIN_DIR)/programmer: $(PROGRAMMER_DIR)/programmer.cc
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/programmer -lwiringPi $(PROGRAMMER_DIR)/programmer.cc

$(BIN_DIR)/programmer_spi: $(PROGRAMMER_DIR)/programmer_spi.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/programmer_spi -lwiringPi $(PROGRAMMER_DIR)/programmer_spi.cpp

$(BIN_DIR)/console: $(PROGRAMMER_DIR)/console.cc
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/console -lwiringPi $(PROGRAMMER_DIR)/console.cc

$(BIN_DIR)/make_screen_image: $(PROGRAMMER_DIR)/make_screen_image.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/make_screen_image $(PROGRAMMER_DIR)/make_screen_image.cpp

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

fetch_roms: $(TOOLS_DIR)/fetch_roms.py $(MEMORY_SET)
	mkdir -p $(ROMS_DIR)
	#cd $(BIN_DIR); python ../$(TOOLS_DIR)/fetch_roms.py $(MEMORY_SET) $(BASEURL)
	cd $(ROMS_DIR); python ../$(TOOLS_DIR)/fetch_roms.py $(MEMORY_SET) $(BASEURL)

$(BIN_DIR)/webserver: $(TOOLS_DIR)/webserver.cpp $(BIN_DIR)/console $(BIN_DIR)/make_screen_image
	g++ -o $(BIN_DIR)/webserver $(TOOLS_DIR)/webserver.cpp

.PHONY: webserver
webserver: $(BIN_DIR)/webserver
	bin/webserver

# FPGA

$(BIN_DIR)/memorymap.bin: $(MEMORY_SET) $(BIN_DIR)/build_memory_map_set $(BIN_DIR)/random_test.bin $(BIN_DIR)/testrom.out $(BIN_DIR)/testrom_appleii.out $(BIN_DIR)/testromv2.out $(BIN_DIR)/ramtest.bin $(BIN_DIR)/nop.bin $(BIN_DIR)/ieee_test.bin
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/build_memory_map_set -d $(ROMS_DIR)/ < $(MEMORY_SET) > $(BIN_DIR)/memorymap.bin

$(BIN_DIR)/enable_table.txt: $(BIN_DIR)/build_enable_table $(ENABLE_TABLE)
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/build_enable_table $(ENABLE_TABLE) > $(BIN_DIR)/enable_table.txt

$(BIN_DIR)/crc32_table.txt: $(BIN_DIR)/crc32
	$(BIN_DIR)/crc32 -t -x > $(BIN_DIR)/crc32_table.txt

$(BIN_DIR)/hardware.bin: $(ROMULATOR_DIR)/*.v $(BIN_DIR)/enable_table.txt $(BIN_DIR)/crc32_table.txt
	mkdir -p $(BIN_DIR)
	cd $(ROMULATOR_DIR); rm hardware.*; apio build
	cp $(ROMULATOR_DIR)/hardware.bin $(BIN_DIR)/hardware.bin
	rm $(ROMULATOR_DIR)/hardware.*

.PHONY: romulator
romulator: $(BIN_DIR)/romulator.bin

$(BIN_DIR)/romulator.bin: $(BIN_DIR)/makerom $(BIN_DIR)/hardware.bin $(BIN_DIR)/memorymap.bin
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/makerom $(BIN_DIR)/hardware.bin $(BIN_DIR)/memorymap.bin > $(BIN_DIR)/romulator.bin

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

$(BIN_DIR)/ieee_test.bin: testrom/ieee_test.c testrom/preinit.s testrom/ieee_test.cfg
	cd testrom; make ieee_test.bin; cp ieee_test.bin ../$(BIN_DIR)/ieee_test.bin; rm ieee_test.bin

$(BIN_DIR)/nop.bin: testrom/nop.c
	cd testrom; make nop.bin; cp nop.bin ../$(BIN_DIR)/nop.bin; rm nop.bin

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