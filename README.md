# bf-romulator
Romulator - RAM/ROM replacement and debug for 6502 CPU systems

romulator - verilog code to implement ROMulator RAM/ROM replacement

programmer - programmer and console application for updating ROMulator, and debug

tools - C++ tools for building firmware images for ROMulator

## About

The ROMulator is a RAM/ROM replacement device for 6502 systems, with programmable memory maps and debug functionality which allows you to halt a running CPU and read from or write to memory.


## Default Switch Settings

The ROMulator supports up to 16 memory maps. Only 4 are populated by default, but you can add any maps you want via the programming interface.
The default switch settings are as follows:

|Setting            |Switch 1   |Switch 2   |Switch 3   |Switch 4   |
|-------------------|-----------|-----------|-----------|-----------|
|BASIC 2, NON-CRTC  |Off        |Off        |Off        |Off        |
|BASIC 4, NON-CRTC      |On         |Off        |Off        |Off        |
|BASIC 4, CRTC, 80C |Off        |On         |Off        |Off        |
|BASIC 4, CRTC, 40C |On         |On         |Off        |Off        |


## Program / Debug

Programming or Debug functionality of the ROMulator uses a Raspberry Pi using a soft SPI implementation. 
You can use the optional RPI ROMulator programming board, or directly connect the following RPI physical pins to the corresponding pins on the 10-pin header on the ROMulator FPGA board:


|RPI     |FPGA Hdr|        |RPI |
|--------|--------|--------|----|
|35      |1       |2       |1   |
|37      |3       |4       |18  |
|40      |5       |6       |NC  |
|36      |7       |8       |NC  |
|35      |9       |10      |34  |
