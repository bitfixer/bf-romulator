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
MEMORY_SET := $(shell pwd)/$(TOOLS_DIR)/default_memory_set.csv
ENABLE_TABLE := $(shell pwd)/$(TOOLS_DIR)/enable_table_pet.csv
BIN_DIR := bin

# Programmer

$(BIN_DIR)/programmer: $(PROGRAMMER_DIR)/programmer.cc
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/programmer -lwiringPi $(PROGRAMMER_DIR)/programmer.cc

$(BIN_DIR)/console: $(PROGRAMMER_DIR)/console.cc
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/console -lwiringPi $(PROGRAMMER_DIR)/console.cc


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

fetch_roms: $(TOOLS_DIR)/fetch_roms.py $(MEMORY_SET)
	mkdir -p $(BIN_DIR)
	cd $(BIN_DIR); python ../$(TOOLS_DIR)/fetch_roms.py $(MEMORY_SET) $(BASEURL)


# FPGA

$(BIN_DIR)/memorymap.bin: $(MEMORY_SET) $(BIN_DIR)/build_memory_map_set
	mkdir -p $(BIN_DIR)
	cd $(BIN_DIR); ./build_memory_map_set < $(MEMORY_SET) > memorymap.bin

$(BIN_DIR)/enable_table.txt: $(BIN_DIR)/build_enable_table $(ENABLE_TABLE)
	mkdir -p $(BIN_DIR)
	$(BIN_DIR)/build_enable_table $(ENABLE_TABLE) > $(BIN_DIR)/enable_table.txt

$(BIN_DIR)/hardware.bin: $(ROMULATOR_DIR)/*.v $(BIN_DIR)/enable_table.txt
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
program: $(BIN_DIR)/romulator.bin $(BIN_DIR)/programmer
	$(BIN_DIR)/programmer -f < $(BIN_DIR)/romulator.bin

.PHONY: reset
reset: $(BIN_DIR)/programmer
	$(BIN_DIR)/programmer -b

clean:
	rm $(BIN_DIR)/*