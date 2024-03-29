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
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <vector>

#define PI_ICE_MISO       21
#define PI_ICE_CLK        23
#define PI_ICE_CDONE      11

#define PI_ICE_CRESET     22
#define PI_ICE_MOSI       19
#define PI_ICE_CS         24


void ice_reset()
{
    pinMode(PI_ICE_CRESET,      OUTPUT);
    digitalWrite(PI_ICE_CRESET, LOW);
    delayMicroseconds(1000);
}

void reset_inout()
{
    pinMode(PI_ICE_CLK,     INPUT);
    pinMode(PI_ICE_CDONE,   INPUT);
    pinMode(PI_ICE_MOSI,    INPUT);
    pinMode(PI_ICE_MISO,    INPUT);
    pinMode(PI_ICE_CRESET,  INPUT);
    pinMode(PI_ICE_CS,      INPUT);
}

int get_time_ms()
{
    return millis();
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

    fprintf(stderr, "flash id:");
    for (int i = 0; i < 20; i++)
        fprintf(stderr, " %02x", buffer[i]);
    fprintf(stderr, "\n");
}

void read_flashmem(int n)
{
    // connect flash to Raspi
    delayMicroseconds(100);
    
    // power_up
    power_up();

    // read flash id
    read_flash_id();
    
    fprintf(stderr, "reading %.2fkB..\n", double(n) / 1024);
    for (int addr = 0; addr < n; addr += 256) {
        uint8_t buffer[256];
        flash_read(addr, buffer, std::min(256, n - addr));
        fwrite(buffer, std::min(256, n - addr), 1, stdout);
    }
    
    // power_down
    power_down();
}

void prog_flashmem(int pageoffset)
{
    delayMicroseconds(100);
    
    // power_up
    power_up();
    
    // read flash id
    read_flash_id();

    // load prog data into buffer
    std::vector<uint8_t> prog_data;
    while (1) {
        int byte = getchar();
        if (byte < 0)
            break;
        prog_data.push_back(byte);
    }
    
    int ms_timer = 0;
    fprintf(stderr, "writing %.2fkB..", double(prog_data.size()) / 1024);
    
    for (int addr = 0; addr < int(prog_data.size()); addr += 256)
    {
        if (addr % (64*1024) == 0)
        {
            fprintf(stderr, "\n%3d%% @%06x ", 100*addr/int(prog_data.size()), addr);
            fprintf(stderr, "erasing 64kB sector..");
            
            flash_write_enable();
            flash_erase_64kB(addr + pageoffset * 0x10000);
            ms_timer += flash_wait();
        }
        
        if (addr % (32*256) == 0) {
            fprintf(stderr, "\n%3d%% @%06x writing: ", 100*addr/int(prog_data.size()), addr);
        }
        
        int n = std::min(256, int(prog_data.size()) - addr);
        uint8_t buffer[256];
        
        bool write_success = false;
        for (int retry_count = 0; retry_count < 100; retry_count++)
        {
            flash_write_enable();
            flash_write(addr + pageoffset * 0x10000, &prog_data[addr], n);
            ms_timer += flash_wait();
            
            flash_read(addr + pageoffset * 0x10000, buffer, n);

            if (!memcmp(buffer, &prog_data[addr], n)) {
                fprintf(stderr, "o");
                //goto written_ok;
                write_success = true;
                break;
            }
            
            fprintf(stderr, "X");
        }
        
        if (!write_success) {
            // restart erasing and writing this 64kB sector
            addr -= addr % (64*1024);
            addr -= 256;
        }
    }
    
    fprintf(stderr, "\n100%% total wait time: %d ms\n", ms_timer);
    power_down();
}

void init_spi()
{
    // make sure spi pins are using the spi function
    system("gpio mode 12 alt0");
    system("gpio mode 13 alt0");
    system("gpio mode 14 alt0");
    system("gpio mode 10 out");
}

int main(int argc, char** argv) {

    int opt;
    bool program = true;
    bool reset = false;
    int size = 0;
    while ((opt = getopt(argc, argv, "pr:b")) != -1)
    {
        switch (opt)
        {
            case 'p':
                program = true;
                break;
            case 'r':
                size = atoi(optarg);
                program = false;
                break;
            case 'b':
                reset = true;
                program = false;
                break;
            default:
                break;
        }
    }


    init_spi(); // set mode of SPI pins
    // setup wiring pi SPI
    int r = wiringPiSetupPhys();
    int fd = wiringPiSPISetup(0, 16000000);
    ice_reset();

    if (program)
    {
        prog_flashmem(0);
    }
    else if (reset)
    {
        // reset fpga
        delay(100);
    }
    else
    {
        read_flashmem(size);
    }
    reset_inout();
}