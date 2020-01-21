#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define PI_ICE_MOSI       24
#define PI_ICE_CLK        27

#define PI_ICE_CRESET     29
#define PI_ICE_MISO       28
#define PI_ICE_CS          5

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
        delayMicroseconds(3);
        digitalWrite(PI_ICE_CLK, LOW);
        delayMicroseconds(3);
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
    delayMicroseconds(5);
    uint32_t res = spi_xfer(data, 8);
    spi_end();

    fprintf(stderr, "SPI send %X got: %X\n", data, res);

    uint8_t d = res;
    return res;
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

    while ((opt = getopt(argc, argv, "rcw:")) != -1)
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
        }
    }

    wiringPiSetup();
    reset_inout();

    pinMode(PI_ICE_CS,      OUTPUT);
    digitalWrite(PI_ICE_CS, HIGH);
    pinMode(PI_ICE_CLK,     OUTPUT);
    digitalWrite(PI_ICE_CLK, LOW);

    if (a == READ)
    {
        // send command to halt CPU
        xfer(0xAA);
        delay(1);

        // send command to read memory map
        xfer(0x66);
        delay(1);
        for (uint32_t i = 0; i < 65536; i++)
        {
            uint8_t byte = xfer(i);
            {
                fwrite(&byte, 1, 1, stdout);
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
}