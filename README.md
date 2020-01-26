# bf-romulator
Romulator - RAM/ROM replacement and debug for 6502 CPU systems

romulator - verilog code to implement ROMulator RAM/ROM replacement

programmer - programmer and console application for updating ROMulator, and debug

tools - C++ tools for building firmware images for ROMulator

## Program / Debug

Programming or Debug functionality of the ROMulator uses a Raspberry Pi using a soft SPI implementation. 
You can use the optional RPI ROMulator programming board, or directly connect the following RPI physical pins to the corresponding pins on the 10-pin header on the ROMulator FPGA board:


RPI     FPGA Hdr        RPI
35      1       2       1
37      3       4       18
40      5       6       NC
36      7       8       NC
35      9       10      34
