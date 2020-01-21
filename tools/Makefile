*.list: *.hex
	xxd -ps @< 

build_memory_map: build_memory_map.cpp
	g++ -o build_memory_map build_memory_map.cpp

build_memory_map_set: build_memory_map_set.cpp
	g++ -o build_memory_map_set build_memory_map_set.cpp

build_enable_table: build_enable_table.cpp
	g++ -o build_enable_table build_enable_table.cpp

enable_table.txt: build_enable_table enable_table_pet.csv
	./build_enable_table enable_table_pet.csv > enable_table.txt

memorymap.bin: default_memory_set.csv build_memory_map_set
	./build_memory_map_set < default_memory_set.csv > memorymap.bin