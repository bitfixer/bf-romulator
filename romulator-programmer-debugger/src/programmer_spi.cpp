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
//#include <wiringPi.h>
//#include <wiringPiSPI.h>
#include <vector>
#include <Arduino.h>
#include <SPI.h>
#include <LittleFS.h>

//#define PI_ICE_MISO       21
//#define PI_ICE_CLK        23
//#define PI_ICE_CDONE      11

//#define PI_ICE_CRESET     22
//#define PI_ICE_MOSI       19
//#define PI_ICE_CS         24

#define  PI_ICE_CRESET  5
#define  PI_ICE_CS      15

#define  LED_PIN        2

unsigned char _inBuffer[512];

void ice_reset()
{
    pinMode(PI_ICE_CRESET,      OUTPUT);
    digitalWrite(PI_ICE_CRESET, LOW);
}

void reset_inout()
{
    //pinMode(PI_ICE_CLK,     INPUT);
    //pinMode(PI_ICE_CDONE,   INPUT);
    //pinMode(PI_ICE_MOSI,    INPUT);
    //pinMode(PI_ICE_MISO,    INPUT);
    pinMode(PI_ICE_CRESET,  INPUT);
    pinMode(PI_ICE_CS,      INPUT);
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

    /*
    // load prog data into buffer
    std::vector<uint8_t> prog_data;
    while (1) {
        int byte = fp.read();
        if (byte < 0)
            break;
        prog_data.push_back(byte);
    }
    */
    
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
    Serial.printf("p to program\n");
    Serial.printf("r to read\n");
    Serial.printf("b to reset\n");
    Serial.printf("--------------\n");
}

void do_command(unsigned char opt)
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
        default:
            break;
    }


    init_spi(); // set mode of SPI pins
    // setup wiring pi SPI
    Serial.printf("resetting\n");
    ice_reset();

    if (program)
    {
        Serial.printf("program\n");
        prog_flashmem(0);
    }
    else if (reset)
    {
        // reset fpga
        Serial.printf("reset\n");
        delay(100);
    }
    else
    {
        Serial.printf("read\n");
        read_flashmem(size);
    }
    reset_inout();

    Serial.printf("done.\n");
}


void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, 0);

    LittleFS.begin();
    display_menu();

    // display start and end addresses
    //Serial.printf("start: %X\n", (uint32_t)romulator_start);
    //Serial.printf("end: %X\n", (uint32_t)romulator_end);
}

void loop() {
    unsigned char b;
    if (Serial.readBytes(&b, 1) > 0) {
        // check for command
        do_command(b);
        display_menu();
    }
}