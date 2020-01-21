hardware.bin: *.v enable_table.txt
	rm hardware.*
	apio build

romulator.bin: hardware.bin memorymap.bin
	../tools/makerom hardware.bin memorymap.bin > romulator.bin