#include "libRomulatorProgrammer.h"
#include "libRomulatorDebug.h"
#include <Arduino.h>
#include <SPI.h>
#include "defines.h"
#include <vector>

void RomulatorProgrammer::initSPI()
{
    pinMode(        PI_ICE_CS, OUTPUT);
    digitalWrite(   PI_ICE_CS, 1);
    SPI.begin();
}

void RomulatorProgrammer::iceReset()
{
    pinMode(        PI_ICE_CRESET, OUTPUT);
    digitalWrite(   PI_ICE_CRESET, LOW);
}

void RomulatorProgrammer::SPIDataRW(uint8_t* data, int len)
{
    digitalWrite(PI_ICE_CS, 0);
    SPI.transferBytes(data, _inBuffer, len);
    memcpy(data, _inBuffer, len);
    digitalWrite(PI_ICE_CS, 1);
}

void RomulatorProgrammer::powerUp()
{
    uint8_t cmd = 0xab;
    SPIDataRW(&cmd, 1);
}

void RomulatorProgrammer::powerDown()
{
    uint8_t cmd = 0xb9;
    SPIDataRW(&cmd, 1);
}

void RomulatorProgrammer::readFlashID()
{
    uint8_t buffer[21];
    memset(buffer, 0, 21);
    buffer[0] = 0x9f;
    SPIDataRW(buffer, 21);

    Serial.printf("flash id:");
    for (int i = 0; i < 20; i++)
        Serial.printf(" %02x", buffer[i]);
    Serial.printf("\n");
}

void RomulatorProgrammer::reset() 
{
    pinMode(PI_ICE_CRESET,      OUTPUT);
    digitalWrite(PI_ICE_CRESET, LOW);
}

void RomulatorProgrammer::flashWriteEnable()
{
    uint8_t cmd = 0x06;
    SPIDataRW(&cmd, 1);
}

void RomulatorProgrammer::flashErase64kB(int addr)
{
    uint8_t buffer[4];
    buffer[0] = 0xd8;
    buffer[1] = addr >> 16;
    buffer[2] = addr >> 8;
    buffer[3] = addr;
    SPIDataRW(buffer, 4);
}

void RomulatorProgrammer::flashWrite(int addr, uint8_t* data, int n)
{
    uint8_t* buffer = new uint8_t[n + 4];
    memset(buffer, 0, n+4);
    buffer[0] = 0x02;
    buffer[1] = addr >> 16;
    buffer[2] = addr >> 8;
    buffer[3] = addr;

    memcpy(&buffer[4], data, n);
    SPIDataRW(buffer, n+4);
    delete[] buffer;
}

void RomulatorProgrammer::flashRead(int addr, uint8_t* data, int n)
{
    uint8_t* buffer = new uint8_t[n + 4];
    memset(buffer, 0, n+4);
    buffer[0] = 0x03;
    buffer[1] = addr >> 16;
    buffer[2] = addr >> 8;
    buffer[3] = addr;

    SPIDataRW(buffer, n+4);
    memcpy(data, &buffer[4], n);
    delete[] buffer;
}

int RomulatorProgrammer::flashWait()
{
    int ms_start = millis();
    
    while (1)
    {
        uint8_t buffer[2];
        buffer[0] = 0x05;
        buffer[1] = 0;

        SPIDataRW(buffer, 2);
        int status = buffer[1];

        if ((status & 0x01) == 0)
            break;
        
        delayMicroseconds(1000);
    }
    
    return millis() - ms_start;
}

void RomulatorProgrammer::beginProgramming(int totalSize)
{
    initSPI();
    iceReset();
    delayMicroseconds(100);
    powerUp();
    readFlashID();

    if (totalSize == 0)
    {
        totalSize = 1179648;
    }

    Serial.printf("writing %.2fkB..", double(totalSize) / 1024);
    _size = totalSize;
    _addr = 0;
    _pageOffset = 0;
    _msTimer = 0;
}

bool RomulatorProgrammer::programBlock(uint8_t* block, int blockSize)
{
    if (_addr % (64*1024) == 0)
    {
        Serial.printf("\n%3d%% @%06x ", 100*_addr/_size, _addr);
        Serial.printf("erasing 64kB sector..");
        
        flashWriteEnable();
        flashErase64kB(_addr + _pageOffset * 0x10000);
        _msTimer += flashWait();
    }

    if (_addr % (32*256) == 0) 
    {
        Serial.printf("\n%3d%% @%06x writing: ", 100*_addr/_size, _addr);
    }

    int n = std::min(256, _size - _addr);
    uint8_t buffer[256];

    bool write_success = false;
    for (int retry_count = 0; retry_count < 100; retry_count++)
    {
        flashWriteEnable();
        flashWrite(_addr + _pageOffset * 0x10000, block, n);
        _msTimer += flashWait();
        
        flashRead(_addr + _pageOffset * 0x10000, buffer, n);

        if (!memcmp(buffer, block, n)) {
            Serial.printf("o");
            //goto written_ok;
            write_success = true;
            break;
        }
        
        Serial.printf("X");
    }

    if (!write_success)
    {
        return false;
    }

    _addr += 256;
    return true;
}

void RomulatorProgrammer::endProgramming()
{
    Serial.printf("\n100%% total wait time: %d ms\n", _msTimer);
    powerDown();
    SPI.end();
    romulatorSetInput();
}