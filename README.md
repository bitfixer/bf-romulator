# bf-romulator
Romulator - RAM/ROM replacement and debug for 6502 CPU systems

romulator - verilog code to implement ROMulator RAM/ROM replacement

programmer - programmer and console application for updating ROMulator, and debug

tools - C++ tools for building firmware images for ROMulator

## About

The ROMulator is a RAM/ROM replacement device for 6502 systems, with programmable memory maps and debug functionality which allows you to halt a running CPU and read from or write to memory. See https://bitfixer.com/romulator for more information.

## Installation

To use the ROMulator, first remove the 6502 CPU from its socket on the PET (or other computer)'s motherboard.
**Note the orientation of the notch on the 6502 chip in the socket.**
Insert the 6502 chip into the 40-pin socket on the ROMulator board. Make sure the notch on the chip is aligned with the notch on the ROMulator socket.
Then, take the ROMulator board, and using the pins on the underside of the board, insert it into the 6502 socket on the motherboard.
**Make sure the notch on the 6502 chip is pointing in its original direction.**
If the ROMulator is inserted backwards, this may damage the ROMulator board since 5v will be presented where it shouldn't be.
Just make sure to double check:
- Original orientation of 6502
- 6502 chip notch should match with notch in ROMulator socket
- When inserting ROMulator into 6502 socket, 6502 chip should be in its original orientation


## Default Switch Settings

The ROMulator supports up to 16 memory maps. Only 4 are populated by default, but you can add any maps you want via the programming interface.
The default switch settings are as follows:

|Set Index  |Setting                                        |Switch 1   |Switch 2   |Switch 3   |Switch 4   |
|-----------|-----------------------------------------------|-----------|-----------|-----------|-----------|
|0          |BASIC 2, NON-CRTC, Business Kbd  (i.e. 2001)   |Off        |Off        |Off        |Off        |
|1          |BASIC 4, NON-CRTC  (i.e. 2001)                 |On         |Off        |Off        |Off        |
|2          |BASIC 4, CRTC, 80C (i.e. 8032)                 |Off        |On         |Off        |Off        |
|3          |BASIC 4, CRTC, 40C (i.e. 4032,4016)            |On         |On         |Off        |Off        |
|4          |Reserved                                       |Off        |Off        |On         |Off        |
|5          |BASIC 2, NON-CRTC, Normal Kbd (i.e. 2001)      |On         |Off        |On         |Off        |

The memory maps are defined in [tools/default_memory_set.csv](tools/default_memory_set.csv).
This file indicates which ROMs are located at which memory address.

## Program / Debug

Programming or Debug functionality of the ROMulator uses a Raspberry Pi using a soft SPI implementation. 
You can use the optional RPI ROMulator programming board, or directly connect the following RPI physical pins to the corresponding pins on the 10-pin header on the ROMulator FPGA board:


|RPI     |FPGA Hdr|FPGA Hdr |RPI |
|--------|--------|---------|----|
|19      |1 (MOSI)|2 (3.3v) |17  |
|24      |3 (CS)  |4 (DBG)  |36  |
|22      |5 (RST) |6 (CDONE)|11  |
|23      |7 (SCK) |8        |NC  |
|21      |9 (MISO)|10 (GND) |34  |

If you are only programming and not running the debug client, DBG and CDONE are optional.
Take a look at this wiring diagram for a visual aid:
![programming/debug diagram](/schematics/programming_wiring.png "programming/debug diagram")

Please follow this procedure to enter debug mode on the romulator and read the contents of a running CPU.
1. run ```make bin/console``` from bf-romulator directory
2. run ```make debug``` to set appropriate pin directions on the pi. Specifically, this sets the DBG line to 1 to allow the ROMulator to start and not be stuck in a halt state, and sets the RST line to an input to avoid holding the fpga in reset.
3. If you are using the optional RPI programming board, remove the programming jumper.
4. run ```bin/console -r > output.bin``` to halt the cpu and read the full memory map into a file called output.bin.

### Installation

To install the ROMulator software on a Raspberry PI for programming or debug, download the script [setup.sh](https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup.sh) onto your Pi and execute from your home directory.
This will install the prequisites and fetch the latest version of the code in this repo.

The one-liner for this on the command line is:\
```wget -O - https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup.sh | sudo bash```

### Programming

To program a new firmware onto the ROMulator, run\
```make program```\
from the ~/bf-romulator directory.

Note that you will need all of the roms present in order to complete the build.\
This means that every unique rom specified in [tools/default_memory_set.csv](tools/default_memory_set.csv) to be present in the bf-romulator/roms directory.\
For convenience if you are downloading ROM images from a single source, there is a script which will do this for you.\
If you were downloading every rom with the base url of http://bitfixer.com, you would run the command:\
```BASEURL=http://bitfixer.com make fetch_roms```\
This downloads every unique rom specified in [tools/default_memory_set.csv](tools/default_memory_set.csv) to the roms directory.
