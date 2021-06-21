# bf-romulator : Tools

This directory contains code which is used to build memory maps for the ROMulator.

Using the ROMulator in any 6502 machine requires configuration of two things: A memory map, and an enable table.

## Memory Maps

A memory map is comprised of a list of ROM files along with memory addresses where these ROMs should be placed in memory.\
Each entry in the memory map in referred to by a number, what we call the set index. Let's look at a couple of examples in the default memory map used by the ROMulator.

### default_memory_set.csv:

This file contains a list of which ROM images should be placed at specified memory addresses in memory maps for the ROMulator.
The format is:

|Set Index	|ROM Filename	|Memory Address in hex i.e. 0xFFFF	|

Let's look at an example - the memory map for a PET 4032.\
Here's the section in the default_memory_set.csv file for this entry:\

```
# PET 4032
4,basic-4-b000.901465-23.bin,0xb000
4,basic-4-c000.901465-20.bin,0xc000
4,basic-4-d000.901465-21.bin,0xd000
4,edit-4-n.901447-29.bin,0xe000
4,kernal-4.901465-22.bin,0xf000
```

This entry has the set index 4. This number maps to a switch setting on the ROMulator's dip switches. Each switch is a binary digit, starting with switch 1 as the least significant bit.\
So index 4 equals binary 0100. The 1s digit maps to switch 1, the 2s digit maps to switch 2, etc. Index 4 would be Switch 1 OFF, Switch 2 OFF, Switch 3 ON, Switch 4 OFF.\

The second column in the entry is the name of a binary file with the contents of a ROM that should be loaded. These files should be located in the #roms# directory.\

the third column is a memory address in hex indicating the start addres of this particular ROM.\

Currently the ROMulator supports up to 16 memory maps (indices 0-15). With an upcoming firmware update, ROMulators with 8 switches on the board will be able to select up to 32. 
