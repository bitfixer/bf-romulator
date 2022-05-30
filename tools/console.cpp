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
#include "constants.h"
#include "libRomulatorDebug.h"
#include <wiringPi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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

bool _halted;
const char* configDir = "config";
const char* profile = "default";
const char* romDir = "roms";

typedef enum _action
{
    READ,
    WRITE,
    CONFIG,
    VRAM,
    CHANGECONFIG,
    INTERACTIVE
} Action;

void printMenu()
{
    printf("--ROMULATOR--\n");
    if (_halted)
    {
        printf("r to read memory\n");
        printf("w to write memory\n");
        printf("c to change configuration setting\n");
        printf("h to run cpu\n");
    }
    else
    {
        printf("h to halt cpu\n");
        printf("v to read vram\n");
        printf("c to read config byte\n");
    }
    printf("q to quit\n");
}

char readCommand()
{
    printf("-->");
    char cmd = 0;
    char tmp = 0;
    int r = scanf("%c", &cmd);
    while (cmd == '\n')
    {
        r = scanf("%c", &cmd);
    }

    if (r != 1)
    {
        return -1;
    }

    r = scanf("%c", &tmp);

    return cmd;
}

void changeConfiguration(int configSetting)
{
    if (configSetting > NUMMAPS-1)
    {
        fprintf(stderr, "invalid setting: %d. Can be between 0 and %d\n", configSetting, NUMMAPS-1);
        exit(1);
    }

    fprintf(stderr, "changing romulator setting to %d\n", configSetting);
    
    // open config file
    char configFname[256];
    sprintf(configFname, "%s/memory_set_%s.csv", configDir, profile);
    fprintf(stderr, "config file: %s\n", configFname);

    FILE* fp = fopen(configFname, "rb");
    if (!fp)
    {
        fprintf(stderr, "could not open %s\n", configFname);
        exit(1);
    }
    
    uint8_t buffer[65536];
    // write new configuration byte
    romulatorWriteConfig(configSetting);
    
    // read memory
    bool read_success = romulatorReadMemory(buffer, 5);
    if (!read_success)
    {
        fprintf(stderr, "could not read memory\n");
        romulatorStartCpu();
        romulatorClose();
        exit(1);
    }
    delay(10);

    // change ROM contents for the new setting
    // read ROM settings from config file
    char line[256];
    int setting;
    char rom[128];
    int address;
    char* token;
    uint8_t romBuffer[65536];
    while (fgets(line, 256, fp))
    {
        setting = -1;
        rom[0] = 0;
        address = -1;

        token = strtok(line, ",");
        if (!token)
        {
            continue;
        }

        setting = atoi(token);
        if (setting != configSetting)
        {
            continue;
        }

        token = strtok(NULL, ",");

        if (!token)
        {
            continue;
        }

        strcpy(rom, token);

        token = strtok(NULL, ",");
        if (!token)
        {
            continue;
        }

        fprintf(stderr, "--- %d %s %s\n", setting, rom, token);
        sscanf(token, "0x%X", &address);
        fprintf(stderr, "address %d %X\n", address, address);


        // check for existence of file
        char romFname[256];
        sprintf(romFname, "%s/%s", romDir, rom);
        fprintf(stderr, "opening %s\n", romFname);

        FILE* fpRom = fopen(romFname, "rb");
        if (!fpRom)
        {
            fprintf(stderr, "could not open %s\n", romFname);
            romulatorStartCpu();
            romulatorClose();
            exit(1);
        }


        int bytesRead = fread(romBuffer, 1, 65536, fpRom);
        fprintf(stderr, "read %d bytes\n", bytesRead);
        fclose(fpRom);

        // copy rom contents
        memcpy(&buffer[address], romBuffer, bytesRead);

        fprintf(stderr, "copied %d bytes to address 0x%X\n", bytesRead, address);
    }
    
    fclose(fp);

    // now copy contents back to romulator
    fprintf(stderr, "writing memory.. ");
    romulatorWriteMemory(buffer, true);
    fprintf(stderr, "done.\n");
    delay(10);
}

bool doCommand(char command)
{
    if (command == 'q')
    {
        return false;
    }

    if (_halted)
    {
        if (command == 'r')
        {
            uint8_t memory[65536];
            bool success = romulatorReadMemory(memory, 5);
            if (!success)
            {
                printf("failed to read memory\n");
                return true;
            }

            printf("write contents to file (press enter for 'memory.bin'): ");
            char fname[256];

            char tmp;
            int r = scanf("%c", &tmp);
            if (tmp == '\n')
            {
                sprintf(fname, "memory.bin");
            }
            else
            {
                fname[0] = tmp;
                r = scanf("%s", &fname[1]);
            }

            FILE* fp = fopen(fname, "wb");
            fwrite(memory, 1, 65536, fp);
            fclose(fp);

            printf("memory written to %s\n", fname);
        }
        else if (command == 'w')
        {
            printf("TBD\n");
        }
        else if (command == 'c')
        {
            printf("change to configuration number: ");
            int conf = 0;
            int r = scanf("%d", &conf);
            printf("changing to configuration %d\n", conf);
            changeConfiguration(conf);
        }
        else if (command == 'h')
        {
            printf("running cpu\n");
            romulatorStartCpu();
            _halted = false;
        }
    }
    else
    {
        if (command == 'h')
        {
            romulatorHaltCpu();
            _halted = true;
            printf("cpu halted.\n");
        }
        else if (command == 'v')
        {
            printf("todo: implement vram\n");
        }
        else if (command == 'c')
        {
            uint8_t cfgByte = romulatorReadConfig();
            printf("config: %d\n", cfgByte);
        }
    }

    return true;
}

void interactive()
{
    while(1) 
    {
        printMenu();
        char cmd = readCommand();
        if (!doCommand(cmd))
        {
            break;
        }
        printf("\n");
    }
}

int main(int argc, char** argv)
{
    int opt;
    Action a = INTERACTIVE;
    FILE* fp = NULL;
    bool xfer_whole_buffer = false;
    bool verify = false;
    int configSetting = 0;
    _halted = false;

    while ((opt = getopt(argc, argv, "rcw:bvsg:i")) != -1)
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
            case 'g':
                a = CHANGECONFIG;
                configSetting = atoi(optarg);
                break;
            case 'i':
                a = INTERACTIVE;
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

        romulatorHaltCpu();
        romulatorWriteMemory(send_buffer, true);
        romulatorStartCpu();
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
    else if (a == CHANGECONFIG)
    {
        romulatorHaltCpu();
        changeConfiguration(configSetting);
        romulatorStartCpu();
    }
    else if (a == INTERACTIVE)
    {
        interactive();
    }

    int end = millis();
    romulatorClose();
    float elapsed = (float)(end - start) / 1000.0;
    fprintf(stderr, "transfer took %f seconds.\n", elapsed);
}
