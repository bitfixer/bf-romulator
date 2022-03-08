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

// programmer_spi.cpp
// program FPGA board using hardware SPI on rpi

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "defines.h"
#include "libRomulatorDebug.h"
#include "libRomulatorProgrammer.h"
#include "server.h"
#include <vector>
#include <Arduino.h>
#include <SPI.h>
#include <LittleFS.h>

unsigned char _inBuffer[512];
typedef enum mode
{
    MENU,
    PROGRAMMING,
    DEBUG
} Mode;

Mode _mode;
bool _cpuHalted;
RomulatorProgrammer _programmer;

void ice_reset()
{
    pinMode(PI_ICE_CRESET,      OUTPUT);
    digitalWrite(PI_ICE_CRESET, LOW);
}

int get_time_ms()
{
    return millis();
}

void wiringPiSPIDataRW(int channel, unsigned char* data, int len)
{
    digitalWrite(PI_ICE_CS, 0);
    SPI.transferBytes(data, _inBuffer, len);
    memcpy(data, _inBuffer, len);
    digitalWrite(PI_ICE_CS, 1);
}

int flash_wait()
{
    int ms_start = get_time_ms();
    
    while (1)
    {
        uint8_t buffer[2];
        buffer[0] = 0x05;
        buffer[1] = 0;

        wiringPiSPIDataRW(0, buffer, 2);
        int status = buffer[1];

        if ((status & 0x01) == 0)
            break;
        
        delayMicroseconds(1000);
    }
    
    return get_time_ms() - ms_start;
}

void flash_erase_64kB(int addr)
{
    uint8_t buffer[4];
    buffer[0] = 0xd8;
    buffer[1] = addr >> 16;
    buffer[2] = addr >> 8;
    buffer[3] = addr;
    wiringPiSPIDataRW(0, buffer, 4);
}

void flash_write_enable()
{
    uint8_t cmd = 0x06;
    wiringPiSPIDataRW(0, &cmd, 1);
}

void flash_write(int addr, uint8_t *data, int n)
{
    uint8_t* buffer = new uint8_t[n + 4];
    memset(buffer, 0, n+4);
    buffer[0] = 0x02;
    buffer[1] = addr >> 16;
    buffer[2] = addr >> 8;
    buffer[3] = addr;

    memcpy(&buffer[4], data, n);
    wiringPiSPIDataRW(0, buffer, n+4);
    delete[] buffer;
}

void flash_read(int addr, uint8_t *data, int n)
{
    uint8_t* buffer = new uint8_t[n + 4];
    memset(buffer, 0, n+4);
    buffer[0] = 0x03;
    buffer[1] = addr >> 16;
    buffer[2] = addr >> 8;
    buffer[3] = addr;

    wiringPiSPIDataRW(0, buffer, n+4);
    memcpy(data, &buffer[4], n);
    delete[] buffer;
}

void power_up()
{
    uint8_t cmd = 0xab;
    wiringPiSPIDataRW(0, &cmd, 1);
}

void power_down()
{
    uint8_t cmd = 0xb9;
    wiringPiSPIDataRW(0, &cmd, 1);
}

void read_flash_id()
{
    uint8_t buffer[21];
    memset(buffer, 0, 21);
    buffer[0] = 0x9f;
    wiringPiSPIDataRW(0, buffer, 21);

    Serial.printf("flash id:");
    for (int i = 0; i < 20; i++)
        Serial.printf(" %02x", buffer[i]);
    Serial.printf("\n");
}

void read_flashmem(int n)
{
    Serial.printf("read flash mem, max size %d\n", n);
    // connect flash to Raspi
    delayMicroseconds(100);
    
    Serial.printf("power up\n");
    // power_up
    power_up();

    Serial.printf("read flash id\n");
    // read flash id
    read_flash_id();
    
    Serial.printf("reading %.2fkB..\n", double(n) / 1024);
    for (int addr = 0; addr < n; addr += 256) {
        uint8_t buffer[256];
        flash_read(addr, buffer, std::min(256, n - addr));
        //fwrite(buffer, std::min(256, n - addr), 1, stdout);

        for (int i = 0; i < 32; i++) {
            Serial.printf("%d: %X\n", addr+i, buffer[i]);
        }
    }
    
    // power_down
    power_down();
}

void prog_flashmem(int pageoffset)
{
    // first try to open data file
    File fp = LittleFS.open("/romulator.bin", "r");
    if (!fp)
    {
        Serial.printf("error: could not open romulator.bin\n");
        return;
    }

    delayMicroseconds(100);
    
    // power_up
    power_up();
    
    // read flash id
    read_flash_id();
    
    int ms_timer = 0;
    Serial.printf("writing %.2fkB..", double(fp.size()) / 1024);
    
    for (int addr = 0; addr < int(fp.size()); addr += 256)
    {
        if (addr % (64*1024) == 0)
        {
            Serial.printf("\n%3d%% @%06x ", 100*addr/int(fp.size()), addr);
            Serial.printf("erasing 64kB sector..");
            
            flash_write_enable();
            flash_erase_64kB(addr + pageoffset * 0x10000);
            ms_timer += flash_wait();
        }
        
        if (addr % (32*256) == 0) {
            Serial.printf("\n%3d%% @%06x writing: ", 100*addr/int(fp.size()), addr);
        }
        
        int n = std::min(256, int(fp.size()) - addr);
        uint8_t buffer[256];
        uint8_t file_buffer[256];

        //Serial.printf("write address %06x\n", addr);

        // read into file buffer
        for (int i = 0; i < n; i++) {
            int byte = fp.read();
            if (byte < 0) {
                Serial.printf("failed to read! addr %X", addr+i);
                break;
            }
            file_buffer[i] = byte;
        }
        
        bool write_success = false;
        for (int retry_count = 0; retry_count < 100; retry_count++)
        {
            flash_write_enable();
            flash_write(addr + pageoffset * 0x10000, file_buffer, n);
            ms_timer += flash_wait();
            
            flash_read(addr + pageoffset * 0x10000, buffer, n);

            if (!memcmp(buffer, file_buffer, n)) {
                Serial.printf("o");
                //goto written_ok;
                write_success = true;
                break;
            }
            
            Serial.printf("X");
        }
        
        if (!write_success) {
            // restart erasing and writing this 64kB sector
            addr -= addr % (64*1024);
            addr -= 256;
        }
    }
    
    Serial.printf("\n100%% total wait time: %d ms\n", ms_timer);
    power_down();
}

void init_spi()
{
    // initialize SPI interface
    pinMode(PI_ICE_CS, OUTPUT);
    digitalWrite(PI_ICE_CS, 1);
    SPI.begin();
}

void display_menu() {
    Serial.printf("\n--------------\n");
    if (_mode == MENU)
    {
        Serial.printf("p for programming mode\n");
        Serial.printf("d for debug mode\n");
    }
    else if (_mode == PROGRAMMING)
    {
        Serial.printf("p to program\n");
        Serial.printf("r to read\n");
        Serial.printf("b to reset\n");
        Serial.printf("m to return to menu\n");
    }
    else if (_mode == DEBUG)
    {
        if (_cpuHalted)
        {
            Serial.printf("CPU Halted.\n");
            Serial.printf("r to read data\n");
            Serial.printf("w to write data\n");
            Serial.printf("v to read vram\n");
            Serial.printf("h to run cpu\n");
        }
        else
        {
            Serial.printf("h to halt cpu\n");
            Serial.printf("c to read config\n");
            Serial.printf("m to return to menu\n");
        }
    }
    Serial.printf("--------------\n");
}

void halt_cpu()
{
    Serial.printf("halting\n");
    romulatorInitDebug();
    romulatorHaltCpu();
    _cpuHalted = true;
}

void run_cpu()
{
    romulatorStartCpu();
    _cpuHalted = false;
}

void read_config()
{
    romulatorInitDebug();
    uint8_t config = romulatorReadConfig();
    Serial.printf("config: %X\n", config);
}

void debug_read_data()
{
    uint8_t buffer[1024];

    Serial.printf("start read\n");
    File fp = LittleFS.open("/memory.bin", "w");
    romulatorStartReadMemory();

    uint32_t crc = 0;
    // read full memory map
    for (int i = 0; i < 64; i++)
    {
        Serial.printf("%d..", i);
        romulatorReadMemoryBlock(buffer, 1024);

        crc32(buffer, 1024, &crc);
        fp.write(buffer, 1024);
    }
    fp.close();

    Serial.printf("\n");
    uint32_t recv_crc = romulatorReadMemoryCRC(buffer);
    Serial.printf("finished read, CRC %X\n", recv_crc);
    Serial.printf("calculated crc: %X\n", crc);
}

void debug_command(unsigned char opt)
{
    if (_cpuHalted)
    {
        switch (opt)
        {
            case 'r':
                // read data
                debug_read_data();
                break;
            case 'w':
                // write data
                break;
            case 'v':
                // read vram
                break;
            case 'h':
                // run cpu
                run_cpu();
                break;
            default:
                break;
        }
    }
    else
    {
        switch (opt)
        {
            case 'h':
                // halt cpu
                halt_cpu();
                break;
            case 'c':
                read_config();
                break;
            case 'm':
                _mode = MENU;
                return;
            default:
                break;
        }
    }
}

// 


void programFirmware()
{
   // first try to open data file
    File fp = LittleFS.open("/romulator.bin", "r");
    if (!fp)
    {
        Serial.printf("error: could not open romulator.bin\n");
        return;
    }


    _programmer.beginProgramming(fp.size());

    uint8_t buffer[256];
    size_t bytesRead = fp.readBytes((char*)buffer, 256);
    while (bytesRead > 0)
    {
        Serial.printf("read %d bytes.\n", bytesRead);
        _programmer.programBlock(buffer, bytesRead);
        bytesRead = fp.readBytes((char*)buffer, 256);
    }

    _programmer.endProgramming();
}

void programming_command(unsigned char opt)
{
    bool program = true;
    bool reset = false;
    int size = 0;
    
    switch (opt)
    {
        case 'p':
            program = true;
            break;
        case 'r':
            //size = atoi(optarg);
            size = 1024;
            program = false;
            break;
        case 'b':
            reset = true;
            program = false;
            break;
        case 'm':
            _mode = MENU;
            return;
        default:
            break;
    } 

    if (program)
    {
        Serial.printf("program\n");
        programFirmware();
        //init_spi(); // set mode of SPI pins
        //prog_flashmem(0);
        //SPI.end();
    }
    else if (reset)
    {
        // reset fpga
        Serial.printf("reset\n");
        ice_reset();
        delay(100);
    }
    else
    {
        Serial.printf("read\n");
        init_spi();
        read_flashmem(size);
        SPI.end();
    }
    
    romulatorSetInput();
    Serial.printf("done.\n");
}

void menu_command(unsigned char opt)
{
    switch (opt)
    {
        case 'p':
            _mode = PROGRAMMING;
            return;
        case 'd':
            _mode = DEBUG;
            return;
        default:
            break;
    }
}

void do_command(unsigned char opt)
{
    if (_mode == MENU)
    {
        menu_command(opt);
    }
    else if (_mode == PROGRAMMING)
    {
        programming_command(opt);
    }
    else if (_mode == DEBUG)
    {
        debug_command(opt);
    }
}

void setup() {
    romulatorSetInput();
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, 0);
    
    LittleFS.begin();
    _cpuHalted = false;
    _mode = MENU;

    startServer();
    display_menu();
}

void loop() {
    handleClient();
    unsigned char b;
    if (Serial.readBytes(&b, 1) > 0) {
        // check for command
        do_command(b);
        display_menu();
    }
}