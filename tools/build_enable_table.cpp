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

#define NUMMAPS 16

typedef enum _region
{
    READWRITE = 0b1010,
    WRITETHROUGH = 0b0111,
    READONLY = 0b1001,
    PASSTHROUGH = 0b0101,
    INACTIVE = 0b0000
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

    return INACTIVE;
}

int main(int argc, char** argv)
{
    int granularity = 2048;

    char* infile = argv[1];
    FILE* fp = fopen(infile, "rb");
    char line[256];

    // create table for all memory maps
    uint8_t table[64 * NUMMAPS];
    memset(table, 0, 64 * NUMMAPS);
    
    while (fgets(line, sizeof(line), fp)) {
        //printf("%s", line);

        char* token = strtok(line, ",");
        // get map index

        // parse the map index from the first field
        // if in the format %d-%d, this is an inclusive range of indices
        // if just %d, it's just a single index
        int start_map_index = -1;
        int end_map_index = -1;
        if (strstr(token, "-")) {
            fprintf(stderr, "range\n");
            sscanf(token, "%d-%d", &start_map_index, &end_map_index);
            fprintf(stderr, "end range %d %d\n", start_map_index, end_map_index);
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
        
        // generate enable bytes for each chunk of this region
        region region_type = get_region_type(token);
        for (int map_index = start_map_index; map_index <= end_map_index; map_index++)
        {
            for (int address = addr; address <= end_addr; address += granularity)
            {
                for (int rw = 1; rw >= 0; rw--)
                {
                    fprintf(stderr, "index %d, address %d, rw %d\n", map_index, address, rw);
                    int table_addr = 64 * map_index;
                    // create table address
                    // addr 15:12 are thebottom 4 bits
                    uint16_t addr_high = (address & 0xF000) >> 12;
                    // addr 11 is the high bit
                    addr_high |= (address & 0x0800) >> 7;
                    addr_high |= (uint16_t)rw << 5;

                    // read
                    uint8_t byteval = 0;
                    if (rw == 1) {
                        byteval = (region_type & 0b1100) >> 2;
                    } else {
                        byteval = (region_type & 0b0011);
                    }

                    //printf("address %X addr_high %d %X r %d %X bv %X\n", address, addr_high, addr_high, region_type, region_type, byteval);
                    table[table_addr + addr_high] = byteval;
                }
            }
        }
    }

    // print table
    for (int aa = 0; aa < 64 * NUMMAPS; aa++)
    {
        //printf("table %d %X => %X\n", aa, aa, table[aa]);
        printf("%X\n", table[aa]);
    }
}