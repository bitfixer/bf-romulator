PROGRAMMER_DIR := programmer
TOOLS_DIR := tools
ROMULATOR_DIR := romulator
MEMORY_SET := default_memory_set.csv
ENABLE_TABLE := enable_table_pet.csv
BIN_DIR := bin

# Programmer

$(BIN_DIR)/programmer: $(PROGRAMMER_DIR)/programmer.cc
	g++ -o $(BIN_DIR)/programmer -lwiringPi $(PROGRAMMER_DIR)/programmer.cc

$(BIN_DIR)/console: $(PROGRAMMER_DIR)/console.cc
	g++ -o $(BIN_DIR)/console -lwiringPi $(PROGRAMMER_DIR)/console.cc


# Tools

$(BIN_DIR)/build_memory_map: $(TOOLS_DIR)/build_memory_map.cpp
	g++ -o $(BIN_DIR)/build_memory_map $(TOOLS_DIR)/build_memory_map.cpp

$(BIN_DIR)/build_memory_map_set: $(TOOLS_DIR)/build_memory_map_set.cpp
	g++ -o $(BIN_DIR)/build_memory_map_set $(TOOLS_DIR)/build_memory_map_set.cpp

$(BIN_DIR)/build_enable_table: $(TOOLS_DIR)/build_enable_table.cpp
	g++ -o $(BIN_DIR)/build_enable_table $(TOOLS_DIR)/build_enable_table.cpp

$(BIN_DIR)/makerom: $(TOOLS_DIR)/makerom.cpp
	g++ -o $(BIN_DIR)/makerom $(TOOLS_DIR)/makerom.cpp

fetch_roms: $(TOOLS_DIR)/fetch_roms.py $(TOOLS_DIR)/$(MEMORY_SET)
	cd $(BIN_DIR); python ../$(TOOLS_DIR)/fetch_roms.py ../$(TOOLS_DIR)/$(MEMORY_SET) $(BASEURL)


# FPGA

$(BIN_DIR)/memorymap.bin: $(TOOLS_DIR)/$(MEMORY_SET) $(BIN_DIR)/build_memory_map_set
	cd $(BIN_DIR); ./build_memory_map_set < ../$(TOOLS_DIR)/$(MEMORY_SET) > memorymap.bin

$(BIN_DIR)/enable_table.txt: $(BIN_DIR)/build_enable_table $(TOOLS_DIR)/$(ENABLE_TABLE)
	$(BIN_DIR)/build_enable_table $(TOOLS_DIR)/$(ENABLE_TABLE) > $(BIN_DIR)/enable_table.txt

$(BIN_DIR)/hardware.bin: $(ROMULATOR_DIR)/*.v $(BIN_DIR)/enable_table.txt
	cd $(ROMULATOR_DIR); rm hardware.*; apio build
	cp $(ROMULATOR_DIR)/hardware.bin $(BIN_DIR)/hardware.bin
	rm $(ROMULATOR_DIR)/hardware.*

$(BIN_DIR)/romulator.bin: $(TOOLS_DIR)/makerom $(BIN_DIR)/hardware.bin $(BIN_DIR)/memorymap.bin
	$(TOOLS_DIR)/makerom $(BIN_DIR)/hardware.bin $(BIN_DIR)/memorymap.bin > $(BIN_DIR)/romulator.bin

# General

clean:
	rm $(BIN_DIR)/*