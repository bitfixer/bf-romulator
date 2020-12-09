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
#include <unistd.h>

#define NUMMAPS 16

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
    char rom_prefix[256];
    sprintf(rom_prefix, "");

    int opt;

    while ((opt = getopt(argc, argv, "d:")) != -1)
    {
        switch (opt)
        {
            case 'd':   // specify rom directory
                sprintf(rom_prefix, "%s", optarg);
                break;
        }
    }

    // full set of memory maps
    uint8_t memorymaps[65536 * NUMMAPS];
    memset(memorymaps, 0, 65536 * NUMMAPS);
    char romname[1024];

    while (fgets(line, sizeof(line), fp)) 
    {
        // skip empty lines
        if (strlen(line) < 2)
        {
            continue;
        }

        if (line[0] == '#' ||
            (line[1] == '/' && line[2] == '/'))
        {
            continue;
        }

        char* token = strtok(line, ",");
        
        // first field is set index
        int index = atoi(token);
        sprintf(romname, "%s%s", rom_prefix, strtok(NULL, ","));

        token = strtok(NULL, ",");
        uint32_t address;
        sscanf(token, "0x%X", &address);
        fprintf(stderr, "%d, rom: %s, addr %X\n", index, romname, address);

        // open rom file
        FILE* fp = fopen(romname, "rw");
        if (fp == NULL)
        {
            // could not find rom file
            fprintf(stderr, "Could not find rom: %s\n", romname);
            fprintf(stderr, "run 'make fetch_roms'\n");
            exit(1);
        }


        long sz = getSize(fp);

        uint32_t map_addr = (65536 * index) + address;

        fprintf(stderr, "map addr: %d %d %X\n", index, map_addr, map_addr);

        fread(&memorymaps[map_addr], 1, sz, fp);
        fclose(fp);
    }

    for (int i = 0; i < NUMMAPS; i++)
    {
        uint32_t addr = 65536*i;
        fwrite(&memorymaps[addr], 1, 65536, stdout);
    }
}