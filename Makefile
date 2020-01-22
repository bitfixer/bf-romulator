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

$(BIN_DIR)/build_memory_map: $(TOOLS_DIR)/build_memory_map.cpp
	mkdir -p $(BIN_DIR)
	g++ -o $(BIN_DIR)/build_memory_map $(TOOLS_DIR)/build_memory_map.cpp

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

program: $(BIN_DIR)/romulator.bin $(BIN_DIR)/programmer
	$(BIN_DIR)/programmer -f < $(BIN_DIR)/romulator.bin

clean:
	rm $(BIN_DIR)/*