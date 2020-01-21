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

long getSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return sz;
}

int main(int argc, char** argv)
{
    uint8_t memory_map[65536];
    memset(memory_map, 0, 65536);

    int numRoms = (argc - 1) / 2;
    fprintf(stderr, "num roms: %d\n", numRoms);

    for (int i = 0; i < numRoms; i++)
    {
        char* romFname = argv[1 + 2*i];
        fprintf(stderr, "ROM %d: %s\n", i, romFname);

        // parse start address of this rom
        char* romAddress = argv[2 + 2*i];
        uint16_t addr;
        sscanf(romAddress, "0x%X", &addr);

        fprintf(stderr, "addr: %d %X\n", addr, addr);

        // read rom into specified address
        FILE* fp = fopen(romFname, "rb");
        long sz = getSize(fp);

        fread(&memory_map[addr], 1, sz, fp);
        fclose(fp);
    }

    fwrite(&memory_map[32768], 1, 32768, stdout);
}