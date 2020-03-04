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

// console.cc - debug console for ROMulator

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

// hardware SPI pins
#define PI_ICE_MISO         19
#define PI_ICE_CLK          23
#define PI_ICE_CDONE        11

#define PI_ICE_CRESET       22
#define PI_ICE_MOSI         21
#define PI_DEBUG_CS         36

#include <wiringPi.h>

void spi_begin()
{
    digitalWrite(PI_DEBUG_CS, LOW);
}

void spi_end()
{
    digitalWrite(PI_DEBUG_CS, HIGH);
}

uint32_t spi_xfer(uint32_t data, int nbits = 8)
{
    uint32_t rdata = 0;
    
    for (int i = nbits-1; i >= 0; i--)
    {
        digitalWrite(PI_ICE_MOSI, (data & (1 << i)) ? HIGH : LOW);
        
        if (digitalRead(PI_ICE_MISO) == HIGH)
            rdata |= 1 << i;
        
        digitalWrite(PI_ICE_CLK, HIGH);
        delayMicroseconds(2);
        digitalWrite(PI_ICE_CLK, LOW);
        //delayMicroseconds(1);
    }
    
    return rdata;
}

void reset_inout()
{
    pinMode(PI_ICE_CLK,     INPUT);
    pinMode(PI_ICE_MOSI,    INPUT);
    pinMode(PI_ICE_MISO,    INPUT);
    pinMode(PI_DEBUG_CS,      OUTPUT);
}

uint8_t xfer(uint32_t data)
{
    pinMode(PI_DEBUG_CS,      OUTPUT);
    pinMode(PI_ICE_MOSI,    OUTPUT);
    pinMode(PI_ICE_CLK,     OUTPUT);

    spi_begin();
    //delayMicroseconds(10);
    uint32_t res = spi_xfer(data, 8);
    spi_end();

    //fprintf(stderr, "SPI send %X got: %X\n", data, res);

    uint8_t d = res;
    return res;
}

uint32_t crc32_for_byte(uint32_t r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, int n_bytes, uint32_t* crc) {
    static uint32_t table[0x100];
    if(!*table)
    {
        fprintf(stderr, "generating table\n");
        for(size_t i = 0; i < 0x100; ++i) 
        {
            table[i] = crc32_for_byte(i);
        }
    }

    for(size_t i = 0; i < n_bytes; ++i)
    {
        *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
    }
}

void xfer_buffer(uint8_t* buffer, int size)
{
    spi_begin();
    delayMicroseconds(5);

    for (int i = 0; i < size; i++)
    {
        uint32_t byte = spi_xfer(buffer[i], 8);
        buffer[i] = (uint8_t)byte;
        delayMicroseconds(1);
    }
    spi_end();

    fprintf(stderr, "end transfer of %d bytes\n", size);
}

typedef enum _action
{
    READ,
    WRITE,
    CONFIG
} Action;

int main(int argc, char** argv)
{

    int opt;
    Action a = READ;
    FILE* fp = NULL;
    bool xfer_whole_buffer = false;

    while ((opt = getopt(argc, argv, "rcw:b")) != -1)
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
        // send command to halt CPU
        xfer(0xAA);
        delay(10);

        // send command to read memory map
        xfer(0x66);
        delay(10);

        if (xfer_whole_buffer)
        {
            uint8_t buffer[65536];
            xfer_buffer(buffer, 65536);
            fwrite(buffer, 1, 65536, stdout);
        } 
        else 
        {
            uint8_t buffer[65536];
            for (uint32_t i = 0; i < 65536; i++)
            {
                uint8_t byte = xfer(i);
                buffer[i] = byte;
            }

            fprintf(stderr, "reading crc32\n");
            delay(1);
            // crc32
            uint32_t crc = 0;
            for (uint32_t i = 0; i < 4; i++)
            {
                crc <<= 0;
                uint8_t byte = xfer(i);
                crc += (uint32_t)byte;
            }

            fprintf(stderr, "crc32: %X\n", crc);

            // calculate crc32 of received buffer
            uint32_t calc_crc = 0;
            crc32(buffer, 65536, &calc_crc);
            fprintf(stderr, "calc crc: %X\n", calc_crc);

            fwrite(buffer, 1, 1, stdout);
        }

        xfer(0x55);
        delay(1);
    }
    else if (a == WRITE)
    {
        // send command to halt CPU
        xfer(0xAA);
        delay(1);

        // write memory map
        xfer(0x99);
        delay(1);
        uint8_t send_byte;
        for (uint32_t i = 0; i < 65536; i++)
        {
            fread(&send_byte, 1, 1, fp);
            uint8_t byte = xfer(send_byte);
            fprintf(stderr, "wrote: %X ", i);
        }

        xfer(0x55);
        xfer(0x55);
        delay(1);
    }
    else if (a == CONFIG) 
    {
        // fetch configuration byte
        xfer(0x77);
        delay(1);
        uint8_t byte = xfer(0);
        fprintf(stderr, "config: %X\n", byte);
    }

    reset_inout();
    int end = millis();
    float elapsed = (float)(end - start) / 1000.0;
    fprintf(stderr, "transfer took %f seconds.\n", elapsed);
}