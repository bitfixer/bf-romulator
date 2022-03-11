#include "libRomulatorDebug.h"
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Arduino.h>
#include <LittleFS.h>

// set everything to input. Also deassert debug line,
// so romulator is not stuck in debug mode
void romulatorSetInput()
{
    pinMode(PI_DEBUG_CS,    OUTPUT);
    digitalWrite(PI_DEBUG_CS, HIGH);

    pinMode(PI_ICE_CLK,     INPUT);
    //pinMode(PI_ICE_CDONE,   INPUT);
    pinMode(PI_ICE_MOSI,    INPUT);
    pinMode(PI_ICE_MISO,    INPUT);
    pinMode(PI_ICE_CRESET,  INPUT);
    pinMode(PI_ICE_CS,      INPUT);
}

void romulatorInitDebug()
{
    romulatorSetInput();
    pinMode(PI_ICE_CLK,         OUTPUT);
    digitalWrite(PI_ICE_CLK,    LOW);
}

void romulatorClose()
{
    romulatorSetInput();
}

void romulatorReset()
{
    pinMode(PI_ICE_CRESET,      OUTPUT);
    digitalWrite(PI_ICE_CRESET, LOW);
}

void romulatorResetDevice()
{
    romulatorReset();
    delay(100);
    romulatorSetInput();
}

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
        fprintf(stderr, "generating table\r\n");
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

    fprintf(stderr, "end transfer of %d bytes\r\n", size);
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

uint32_t crcFromFile(File fp)
{
    uint8_t buffer[1024];
    uint32_t crc = 0;
    int bytesRead = fp.readBytes((char*)buffer, 1024);
    while (bytesRead > 0)
    {
        crc32(buffer, bytesRead, &crc);
        bytesRead = fp.readBytes((char*)buffer, 1024);
    }

    fp.seek(0);
    return crc;
}

bool romulatorWriteMemoryFromFile()
{
    File fp = LittleFS.open("/memory.bin", "r");
    if (!fp)
    {
        return false;
    }

    uint32_t calc_crc = crcFromFile(fp);
    fprintf(stderr, "send calc crc: %X\r\n", calc_crc);

    // write memory map
    xfer(0x99);
    
    uint8_t send_byte;
    for (uint32_t i = 0; i < 65536; i++)
    {
        //send_byte = send_buffer[i];
        send_byte = fp.read();
        uint8_t byte = xfer(send_byte);
    }

    fp.seek(0);
    xfer(0x55);

    /*
    if (verify)
    {
        uint8_t buffer[65536];
        if (!romulatorReadMemory(buffer, 5))
        {
            fprintf(stderr, "read error during verify.\r\n");
        }

        if (memcmp(send_buffer, buffer, 65536) != 0)
        {
            fprintf(stderr, "write error, mismatch.\r\n");
        }
    }
    */
   return true;
}

void romulatorStartReadMemory()
{
    xfer(0x66);

    // read dummy byte
    uint8_t b = xfer(0);
    fprintf(stderr, "dummy byte: %X\r\n", b);
}

void romulatorReadMemoryBlock(uint8_t* buf, int size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        uint8_t byte = xfer(i);
        buf[i] = byte;
    }
}

uint32_t romulatorReadMemoryCRC(uint8_t* buf)
{
    // crc32
    uint32_t crc = 0;
    for (uint32_t i = 0; i < 4; i++)
    {
        crc <<= 8;
        uint8_t byte = xfer(i);
        crc += (uint32_t)byte;
    }
    return crc;
}

bool romulatorReadMemoryToFile()
{
    uint8_t buffer[1024];
    Serial.printf("start read\r\n");
    File fp = LittleFS.open("/memory.bin", "w");
    if (!fp)
    {
        return false;
    }
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

    Serial.printf("\r\n");
    uint32_t recv_crc = romulatorReadMemoryCRC(buffer);
    Serial.printf("finished read, CRC %X\r\n", recv_crc);
    Serial.printf("calculated crc: %X\r\n", crc);
    return true;
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

    if (verbose) fprintf(stderr, ": P %02X *LP %02X\r\n", parity_byte, local_parity);

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

bool romulatorReadVram(uint8_t* vram, int size, int valid_bytes, int retries)
{
    int byte = 0;
    start_vram_read();
    bool read_success = true;
    while (byte < size)
    {
        //fprintf(stderr, "byte %d\r\n", byte);
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
                fprintf(stderr, "error byte %d retries %d\r\n", byte, retries);
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