#include "libRomulatorDebug.h"
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// hardware SPI pins
#define PI_ICE_MISO         19
#define PI_ICE_CLK          23
#define PI_ICE_CDONE        11

#define PI_ICE_CRESET       22
#define PI_ICE_MOSI         21
#define PI_DEBUG_CS         36

void reset_inout()
{
    pinMode(PI_ICE_CLK,         INPUT);
    pinMode(PI_ICE_MOSI,        INPUT);
    pinMode(PI_ICE_MISO,        INPUT);
    pinMode(PI_DEBUG_CS,        OUTPUT);
}

void romulatorInit()
{
    wiringPiSetupPhys();
    reset_inout();

    int start = millis();

    pinMode(PI_DEBUG_CS,        OUTPUT);
    digitalWrite(PI_DEBUG_CS,   HIGH);
    pinMode(PI_ICE_CLK,         OUTPUT);
    digitalWrite(PI_ICE_CLK,    LOW);
}

void romulatorClose()
{
    reset_inout();
}

void spi_begin()
{
    digitalWrite(PI_DEBUG_CS, LOW);
}

void spi_end()
{
    digitalWrite(PI_DEBUG_CS, HIGH);
}

void delay_nanos(long nanos)
{
    timespec ts1, ts2;
    long diff_ns;
    clock_gettime(CLOCK_REALTIME, &ts1);
    do {
        clock_gettime(CLOCK_REALTIME, &ts2);
        diff_ns = (ts2.tv_sec - ts1.tv_sec)*1000000000l + (ts2.tv_nsec - ts1.tv_nsec);
    } while (diff_ns < nanos);
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
        delayMicroseconds(1);
        digitalWrite(PI_ICE_CLK, LOW);
    }
    
    return rdata;
}

uint8_t xfer(uint32_t data)
{
    pinMode(PI_DEBUG_CS,      OUTPUT);
    pinMode(PI_ICE_MOSI,    OUTPUT);
    pinMode(PI_ICE_CLK,     OUTPUT);

    spi_begin();
    uint32_t res = spi_xfer(data, 8);
    spi_end();

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

void romulatorHaltCpu()
{
    // send command to halt CPU
    xfer(0xAA);
}

void romulatorStartCpu()
{
    xfer(0x55);
}

void start_vram_read()
{
    xfer(0x88);
}

void romulatorWriteMemory(uint8_t* send_buffer, bool verify)
{
    uint32_t calc_crc = 0;
    crc32(send_buffer, 65536, &calc_crc);
    fprintf(stderr, "send calc crc: %X\n", calc_crc);

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
}

bool romulatorReadMemory(uint8_t* buffer, int retries)
{
    for (int attempt = 0; attempt < retries; attempt++)
    {
        // send command to read memory map
        xfer(0x66);

        // read dummy byte
        uint8_t b = xfer(0);
        fprintf(stderr, "dummy byte: %X\n", b);

        for (uint32_t i = 0; i < 65536; i++)
        {
            uint8_t byte = xfer(i);
            buffer[i] = byte;
        }

        // crc32
        uint32_t crc = 0;
        for (uint32_t i = 0; i < 4; i++)
        {
            crc <<= 8;
            uint8_t byte = xfer(i);
            fprintf(stderr, "r %d %X\n", i, byte);
            crc += (uint32_t)byte;
        }

        fprintf(stderr, "crc32: %X\n", crc);

        // calculate crc32 of received buffer
        uint32_t calc_crc = 0;
        crc32(buffer, 65536, &calc_crc);
        fprintf(stderr, "calc crc: %X\n", calc_crc);

        if (crc == calc_crc)
        {
            return true;
        }

        fprintf(stderr, "attempt %d read failure, crc mismatch\n", attempt);
    }
    return false;
}

bool romulatorReadVramBlock(uint8_t* vram)
{
    bool verbose = false;
    for (int b = 0; b < 8; b++)
    {
        vram[b] = xfer(0);
        if (verbose) fprintf(stderr, "%02X ", vram[b]);
    }

    // receive parity byte
    uint8_t parity_byte = xfer(0);

    // generate local parity byte
    uint8_t local_parity = 0;
    // generate local version of parity byte
    for (int pb = 0; pb < 8; pb++)
    {
        local_parity >>= 1;
        uint8_t parity_bit = 0;
        uint8_t data_byte = vram[pb];
        for (int bit = 0; bit < 8; bit++)
        {
            parity_bit += data_byte & 0x01;
            data_byte >>= 1;
        }

        local_parity |= ((parity_bit & 0x01) << 7);
    }

    if (verbose) fprintf(stderr, ": P %02X *LP %02X\n", parity_byte, local_parity);

    if (parity_byte != local_parity)
    {
        return false;
    }

    return true;
}

uint8_t romulatorReadConfig()
{
    // fetch configuration byte
    xfer(0x77);
    delay(1);
    uint8_t byte = xfer(0);
    return byte;
}

void romulatorWriteConfig(int configSetting)
{
    uint8_t configByte = 0xE0 | (uint8_t)configSetting;
    fprintf(stderr, "writing %X\n", configByte);
    xfer(configByte);
    delay(1);
}

bool romulatorReadVram(uint8_t* vram, int size, int valid_bytes, int retries)
{
    int byte = 0;
    start_vram_read();
    bool read_success = true;
    while (byte < size)
    {
        //fprintf(stderr, "byte %d\n", byte);
        bool success = false;
        // read an 8-byte block from vram
        success = romulatorReadVramBlock(&vram[byte]);
        if (success || byte >= valid_bytes)
        {
            retries = 0;
            xfer(0);
            byte += 8;
        }
        else
        {
            // parity error token
            if (retries < 5)
            {
                retries++;
                fprintf(stderr, "error byte %d retries %d\n", byte, retries);
                xfer(0x22);
            }
            else
            {
                // give up on this block
                retries = 0;
                xfer(0);
                byte += 8;
                read_success = false;
            }
        }
    }

    return read_success;
}