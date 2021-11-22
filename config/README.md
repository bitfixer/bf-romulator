# bf-romulator : Config

This directory contains configuration files which are used to build memory maps for the ROMulator.

Using the ROMulator in any 6502 machine requires configuration of two things: A memory map, and an enable table.

## Memory Maps

A memory map is comprised of a list of ROM files along with memory addresses where these ROMs should be placed in memory.\
Each entry in the memory map in referred to by a number, what we call the set index. Let's look at a couple of examples in the default memory map used by the ROMulator.

### memory_set_default.csv:

This file contains a list of which ROM images should be placed at specified memory addresses in memory maps for the ROMulator.
The format is:

|Set Index	|ROM Filename	|Memory Address in hex i.e. 0xFFFF	|

You can include comments in this file by starting the line with a # character.

Let's look at an example - the memory map for a PET 4032.\
Here's the section in the memory_set_default.csv file for this entry:

```
# PET 4032
4,basic-4-b000.901465-23.bin,0xb000
4,basic-4-c000.901465-20.bin,0xc000
4,basic-4-d000.901465-21.bin,0xd000
4,edit-4-n.901447-29.bin,0xe000
4,kernal-4.901465-22.bin,0xf000
```

This entry has the set index 4. This number maps to a switch setting on the ROMulator's dip switches. Each switch is a binary digit, starting with switch 1 as the least significant bit.\
So index 4 equals binary 0100. The 1s digit maps to switch 1, the 2s digit maps to switch 2, etc. Index 4 would be Switch 1 OFF, Switch 2 OFF, Switch 3 ON, Switch 4 OFF.

The second column in the entry is the name of a binary file with the contents of a ROM that should be loaded. These files should be located in the #roms# directory.

the third column is a memory address in hex indicating the start addres of this particular ROM.\

Currently the ROMulator supports up to 16 memory maps (indices 0-15). With an upcoming firmware update, ROMulators with 8 switches on the board will be able to select up to 32. 

To support a new 6502 machine, first get a list of the ROMs for that machine, and the memory locations where they should be placed. Then you can add an entry to memory_set_default.csv for your configuration. You can also make an entirely separate csv file, this is covered a bit later.

## Enable Tables

The second half of a configuration for the ROMulator is the enable table. This is a list of ranges in memory space specified by a start and end address, and a keyword which indicates how that section of memory should be handled by the ROMulator. 
The default enable table is defined in the following file:

### enable_table_default.csv

Similar to a memory map, each line of the enable table has the format:

|Set Index (or range of indices)|Start Address in Hex|End Address in Hex|"Keyword"|

Let's look at the example for setting 4 again, for the PET 4032.
The first few lines of the enable table file are:
```
0-14,0x0000,0x7FFF,"readwrite"
0-14,0x8000,0x8FFF,"writethrough"
0-14,0x9000,0xAFFF,"passthrough"
0-14,0xB000,0xE7FF,"readonly"
0-14,0xE800,0xEFFF,"passthrough"
0-14,0xF000,0xFFFF,"readonly"
```

Each of these lines has a range of indices specified instead of a single index.\
Examine the first line:
Set indices are 0-14. This means to use this setting for all switch settings between 0-14, inclusive.
Start address is 0x0000, and end address is 0x7FFF. This means the setting applies to this range of addresses. The end address is inclusive.
Then the keyword is "readwrite". This means for this range, we want to use the ROMulator's onboard memory to override any reads or writes to these addresses.

You can specify ranges of indices like this as a shortcut to save you from duplicating the same entries for a series of settings that are the same or nearly the same, as is the case with a group of similar PET computers. 

These lines can be overridden later in the file as is done with this entry:
```
# only enable 16k ram for 4016
7,0x0000,0x3FFF,"readwrite"
7,0x4000,0x7FFF,"passthrough"
```

In this case, for index 7, we override the previously specified keyword for these regions of memory, which effectively disables region 0x4000-0x7FFF on the ROMulator, meaning that the PET 4016 only detects 16k of memory. An example of how you can override range settings.

### memory enable keywords

The following keywords are used in this table:
* "readwrite" : fully replace any read or write to this range. This is used for RAM that you wish the ROMulator to replace.
* "readonly" : replace any read to this range with the ROMulator's memory, and disable writes. This is used for ROMs.
* "passthrough" : reads and writes to this region should go through to the mainboard and not be intercepted by the ROMulator. This is used for I/O.
* "writethrough" : writes to this region are done both to the mainboard and to the ROMulator. Reads are read from the mainboard. This is useful for sections of memory that are read by something other than the CPU, which is often the case with video RAM.

Adding an enable table entry or entries for a new 6502 machine means learning the memory map for that machine and defining the regions of memory directly in enable_table_default.csv or in a separate csv.\
Generally to replace RAM, use "readwrite". To replace ROM, use "readonly". IO regions should be "passthrough" and video ram would usually be "writethrough". Any other section you don't want the ROMulator to touch would be "passthrough".

Currently, the smallest independently configurable region is 256 bytes. This is somewhat arbitrary and could be smaller if needed with a software change, it just uses more block RAM on the FPGA to do so.

Once you have set up your memory map and enable table, and have gotten all your ROM files into the roms directory, then you are ready to load the new firmware with your configuration: See https://github.com/bitfixer/bf-romulator#programming to do this.
