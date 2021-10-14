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

typedef enum _action
{
    READ,
    WRITE,
    CONFIG,
    VRAM
} Action;

void reset_inout()
{
    pinMode(PI_ICE_CLK,     INPUT);
    pinMode(PI_ICE_MOSI,    INPUT);
    pinMode(PI_ICE_MISO,    INPUT);
    pinMode(PI_DEBUG_CS,      OUTPUT);
}

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

    wiringPiSetupPhys();
    reset_inout();

    int start = millis();

    pinMode(PI_DEBUG_CS,      OUTPUT);
    digitalWrite(PI_DEBUG_CS, HIGH);
    pinMode(PI_ICE_CLK,     OUTPUT);
    digitalWrite(PI_ICE_CLK, LOW);

    if (a == READ)
    {
        uint8_t buffer[65536];
        halt_cpu();
        bool read_success = read_memory(buffer, 5);
        start_cpu();

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

        uint32_t calc_crc = 0;
        crc32(send_buffer, 65536, &calc_crc);
        fprintf(stderr, "send calc crc: %X\n", calc_crc);

        // send command to halt CPU
        halt_cpu();

        // write memory map
        xfer(0x99);
        
        uint8_t send_byte;
        for (uint32_t i = 0; i < 65536; i++)
        {
            send_byte = send_buffer[i];
            uint8_t byte = xfer(send_byte);
        }

        xfer(0x55);
        if (verify)
        {
            uint8_t buffer[65536];
            if (!romulatorReadMemory(buffer, 5))
            {
                fprintf(stderr, "read error during verify.\n");
            }

            if (memcmp(send_buffer, buffer, 65536) != 0)
            {
                fprintf(stderr, "write error, mismatch.\n");
            }
        }

        start_cpu();
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
        romulatorReadVram(vram, 1024, valid_bytes);
        fwrite(vram, 1, valid_bytes, stdout);
    }

    reset_inout();
    int end = millis();
    float elapsed = (float)(end - start) / 1000.0;
    fprintf(stderr, "transfer took %f seconds.\n", elapsed);
}