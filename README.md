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

|Set Index  |Setting            |Switch 1   |Switch 2   |Switch 3   |Switch 4   |
|-----------|-------------------|-----------|-----------|-----------|-----------|
|0          |BASIC 2, NON-CRTC  |Off        |Off        |Off        |Off        |
|1          |BASIC 4, NON-CRTC  |On         |Off        |Off        |Off        |
|2          |BASIC 4, CRTC, 80C |Off        |On         |Off        |Off        |
|3          |BASIC 4, CRTC, 40C |On         |On         |Off        |Off        |

The memory maps are defined in [tools/default_memory_set.csv](tools/default_memory_set.csv).

## Program / Debug

Programming or Debug functionality of the ROMulator uses a Raspberry Pi using a soft SPI implementation. 
You can use the optional RPI ROMulator programming board, or directly connect the following RPI physical pins to the corresponding pins on the 10-pin header on the ROMulator FPGA board:


|RPI     |FPGA Hdr|FPGA Hdr|RPI |
|--------|--------|--------|----|
|38      |1 (MOSI)|2 (3.3v)|1   |
|37      |3 (CS)  |4 (DBG) |18  |
|40      |5 (RST) |6       |NC  |
|36      |7 (SCK) |8       |NC  |
|35      |9 (MISO)|10 (GND)|34  |

### Installation

To install the ROMulator software on a Raspberry PI for programming or debug, download the script [setup.sh](https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup.sh) onto your Pi and execute from your home directory.
This will install the prequisites and fetch the latest version of the code in this repo.

The one-liner for this on the command line is:
```wget -O - https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup.sh | bash```

### Programming

To program a new firmware onto the ROMulator, run
```make program```
from the ~/bf-romulator directory.
