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
#define PI_ICE_MISO       12
#define PI_ICE_CLK        14
#define PI_ICE_CDONE      0

#define PI_ICE_CRESET     6
#define PI_ICE_MOSI       13
#define PI_ICE_CS         27

#include <wiringPi.h>

void spi_begin()
{
    digitalWrite(PI_ICE_CS, LOW);
}

void spi_end()
{
    digitalWrite(PI_ICE_CS, HIGH);
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
    pinMode(PI_ICE_CS,      OUTPUT);
}

uint8_t xfer(uint32_t data)
{
    pinMode(PI_ICE_CS,      OUTPUT);
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

    wiringPiSetup();
    reset_inout();

    int start = millis();

    pinMode(PI_ICE_CS,      OUTPUT);
    digitalWrite(PI_ICE_CS, HIGH);
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
            for (uint32_t i = 0; i < 65536; i++)
            {
                uint8_t byte = xfer(i);
                {
                    fwrite(&byte, 1, 1, stdout);
                }
            }
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