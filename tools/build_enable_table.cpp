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

// build_enable_table
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "constants.h"

// define size of minimum memory region which can have its own 
// enable setting.
#define ADDR_GRANULARITY_SIZE 256

#define MAX_VRAM_SIZE 2048

// region definitions for different key strings
// each region is a 5 bit number
// bit 4: is a vram region
// bit 3: read enable for romulator
// bit 2: read enable for mainboard
// bit 1: write enable for romulator
// bit 0: write enable for mainboard
typedef enum _region
{
    READWRITE = 0b01010,
    WRITETHROUGH = 0b00111,
    READONLY = 0b01001,
    PASSTHROUGH = 0b00101,
    VRAM = 0b10111,
    INACTIVE = 0b00000
} region;

region get_region_type(char* region_str) {
    if (strstr(region_str, "readwrite"))
    {
        return READWRITE;
    }
    else if (strstr(region_str, "writethrough"))
    {
        return WRITETHROUGH;
    }
    else if (strstr(region_str, "readonly"))
    {
        return READONLY;
    }
    else if (strstr(region_str, "passthrough"))
    {
        return PASSTHROUGH;
    }
    else if (strstr(region_str, "vram"))
    {
        return VRAM;
    }

    return INACTIVE;
}

int main(int argc, char** argv)
{
    //int granularity = 2048;

    // determine number of necessary entries to 
    // address enable table
    int granularity = ADDR_GRANULARITY_SIZE;

    int num_address_entries = (int)pow(2, 16) / granularity;
    int addr_entry_bits = (int)log2((double)num_address_entries);

    // get number of bits needed to represent the number of maps
    int config_bits = (int)log2((double)NUMMAPS);

    // total bits needed for every entry in address enable table
    int num_entry_bits = addr_entry_bits + config_bits + 1;
    int num_entries = (int)pow(2, num_entry_bits);

    //printf("g %d nae %d aeb %d cb %d neb %d ne %d\n", granularity, num_address_entries, 
    //    addr_entry_bits, config_bits, num_entry_bits, num_entries);

    // open memory enable configuration file
    char* infile = argv[1];
    FILE* fp = fopen(infile, "rb");
    char line[256];

    // create table for all memory maps
    uint8_t* table = new uint8_t[num_entries];
    memset(table, 0, num_entries);

    uint16_t vram_start_addr[NUMMAPS];
    uint16_t vram_end_addr[NUMMAPS];

    for (int i = 0; i < NUMMAPS; i++)
    {
        vram_start_addr[i] = 0;
        vram_end_addr[i] = 0;
    }
    
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

        //printf("%s", line);

        char* token = strtok(line, ",");
        // get map index

        // parse the map index from the first field
        // if in the format %d-%d, this is an inclusive range of indices
        // if just %d, it's just a single index
        int start_map_index = -1;
        int end_map_index = -1;
        if (strstr(token, "-")) {
            sscanf(token, "%d-%d", &start_map_index, &end_map_index);
        } else {
            start_map_index = atoi(token);
            end_map_index = start_map_index;
        }

        //int map_index = atoi(token);

        token = strtok(NULL, ",");
        // start address
        uint32_t addr;
        uint32_t end_addr;
        sscanf(token, "0x%X", &addr);

        //printf("start address: %d %X\n", addr, addr);

        // end address
        token = strtok(0, ",");
        //printf("start address: %d %X\n", addr, addr);
        
        sscanf(token, "0x%X", &end_addr);
        //printf("end address: %d %X %X\n", end_addr, end_addr, addr);

        // region type
        token = strtok(0, ",");
        //printf("region type: %s addr %X end_addr %X\n", token, addr, end_addr);
        
        //int table_addr = 64 * map_index;
        // generate enable bytes for each chunk of this region
        region region_type = get_region_type(token);

        if (region_type == VRAM)
        {
            // VRAM regions are independent of other regions in the memory map,
            // and maximum size (currently) is 2k.

            // check if this VRAM region is the right size
            if (end_addr - addr > MAX_VRAM_SIZE)
            {
                fprintf(stderr, "build_enable_table error: VRAM region (setting %d, start %04X end %04X) exceeds maximum size of %d bytes.\n", 
                    start_map_index, 
                    addr, 
                    end_addr, 
                    MAX_VRAM_SIZE);
                exit(1);
            }

            for (int map_index = start_map_index; map_index <= end_map_index; map_index++)
            {
                vram_start_addr[map_index] = addr;
                vram_end_addr[map_index] = end_addr;
            }
        }
        else
        {
            for (int map_index = start_map_index; map_index <= end_map_index; map_index++)
            {
                for (uint32_t address = addr; address <= end_addr; address += (uint32_t)granularity)
                {
                    for (uint16_t rw = 0; rw < 2; rw++)
                    {
                        // get table address
                        // address has the following bit pattern:
                        // config(config_bits), rw, addr(addr_entry_bits)
                        uint16_t config_index = (uint16_t)map_index;
                        uint16_t table_addr = config_index;
                        table_addr <<= 1;
                        table_addr += rw;
                        table_addr <<= addr_entry_bits;

                        // get high bits of address to get the index of the address entry
                        int addr_shift = 16 - addr_entry_bits;
                        uint16_t entry_addr = (uint16_t)address >> addr_shift;
                        table_addr += entry_addr;

                        // get bit pattern for this entry
                        // 2 higher bits are the read (high) value
                        // 2 lower bits are the write value
                        uint8_t byteval = 0;
                        if (rw == 1) {
                            byteval = (region_type & 0b01100) >> 2;
                        } else {
                            byteval = (region_type & 0b00011);
                        }

                        //fprintf(stderr, "address %X, rw %d, ci %d, as %d, table_addr %d %X, r %d %X bv %X\n", address, rw, config_index, addr_shift, table_addr, table_addr, region_type, region_type, byteval);
                        table[table_addr] = byteval;
                    }
                }
            }
        }
    }

    // print table
    for (int aa = 0; aa < num_entries; aa++)
    {
        printf("%X\n", table[aa]);
    }

    FILE* fp_vram = fopen("bin/vram_start_addr.txt", "wb");
    for (int v = 0; v < NUMMAPS; v++)
    {
        fprintf(fp_vram, "%04X\n", vram_start_addr[v]);
    }
    fclose(fp_vram);

    fp_vram = fopen("bin/vram_end_addr.txt", "wb");
    for (int v = 0; v < NUMMAPS; v++)
    {
        fprintf(fp_vram, "%04X\n", vram_end_addr[v]);
    }
    fclose(fp_vram);

    delete[] table;
}