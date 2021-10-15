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

// console.cpp - debug console for ROMulator
#include "libRomulatorDebug.h"
#include <wiringPi.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// hardware SPI pins
#define PI_ICE_MISO         19
#define PI_ICE_CLK          23
#define PI_ICE_CDONE        11

#define PI_ICE_CRESET       22
#define PI_ICE_MOSI         21
#define PI_DEBUG_CS         36

typedef enum _action
{
    READ,
    WRITE,
    CONFIG,
    VRAM
} Action;

int main(int argc, char** argv)
{
    int opt;
    Action a = READ;
    FILE* fp = NULL;
    bool xfer_whole_buffer = false;
    bool verify = false;

    while ((opt = getopt(argc, argv, "rcw:bvs")) != -1)
    {
        switch (opt)
        {
            case 'r':
                a = READ;
                break;
            case 'w':
                a = WRITE;
                fprintf(stderr, "file: %s\n", optarg);
                fp = fopen(optarg, "rb");
                break;
            case 'c':
                a = CONFIG;
                break;
            case 'b':
                xfer_whole_buffer = true;
                break;
            case 'v':
                verify = true;
                break;
            case 's':
                a = VRAM;
                break;
        }
    }

    romulatorInit();

    int start = millis();

    if (a == READ)
    {
        uint8_t buffer[65536];
        romulatorHaltCpu();
        bool read_success = romulatorReadMemory(buffer, 5);
        romulatorStartCpu();

        if (!read_success)
        {
            fprintf(stderr, "read error\n");
        }

        fwrite(buffer, 1, 65536, stdout);
    }
    else if (a == WRITE)
    {
        uint8_t send_buffer[65536];
        fread(send_buffer, 1, 65536, fp);

        romulatorWriteMemory(send_buffer, true);
    }
    else if (a == CONFIG) 
    {
        uint8_t byte = romulatorReadConfig();
        fprintf(stderr, "config: %X\n", byte);
    }
    else if (a == VRAM)
    {
        // retrieve contents of video ram
        int retries = 0;
        uint8_t vram[1024];
        int valid_bytes = 1000;
        romulatorReadVram(vram, 1024, valid_bytes, 5);
        fwrite(vram, 1, valid_bytes, stdout);
    }

    int end = millis();
    romulatorClose();
    float elapsed = (float)(end - start) / 1000.0;
    fprintf(stderr, "transfer took %f seconds.\n", elapsed);
}