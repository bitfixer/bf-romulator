// ROMulator - RAM/ROM replacement and diagnostic for 8-bit CPUs
// Copyright (C) 2019  Michael Hill

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define NUMMAPS 4

long getSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return sz;
}

int main(int argc, char** argv)
{
    // parse input csv
    FILE* fp = stdin;
    char line[256];

    // full set of memory maps
    uint8_t memorymaps[65536 * NUMMAPS];
    memset(memorymaps, 0, 65536 * NUMMAPS);

    while (fgets(line, sizeof(line), fp)) 
    {
        char* token = strtok(line, ",");
        
        // first field is set index
        int index = atoi(token);
        char* romname = strtok(NULL, ",");

        token = strtok(NULL, ",");
        uint32_t address;
        sscanf(token, "0x%X", &address);
        fprintf(stderr, "%d, rom: %s, addr %X\n", index, romname, address);

        // open rom file
        FILE* fp = fopen(romname, "rw");
        long sz = getSize(fp);

        uint32_t map_addr = (65536 * index) + address;

        fprintf(stderr, "map addr: %d %d %X\n", index, map_addr, map_addr);

        fread(&memorymaps[map_addr], 1, sz, fp);
        fclose(fp);
    }

    for (int i = 0; i < NUMMAPS; i++)
    {
        uint32_t addr = (65536 * i) + 32768;
        fwrite(&memorymaps[addr], 1, 32768, stdout);
    }
}