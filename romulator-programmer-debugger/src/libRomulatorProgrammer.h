#ifndef __LIB_ROMULATOR_PROGRAMMER_H__
#define __LIB_ROMULATOR_PROGRAMMER_H__

#include <stdint.h>

class RomulatorProgrammer 
{
public:
    RomulatorProgrammer() {};
    ~RomulatorProgrammer() {};

    void reset();
    void beginProgramming(int totalSize);
    bool programBlock(uint8_t* block, int blockSize);
    void endProgramming();

private:

    void powerUp();
    void powerDown();
    void flashWriteEnable();
    void flashErase64kB(int addr);
    void flashWrite(int addr, uint8_t* data, int n);
    void flashRead(int addr, uint8_t* data, int n);
    int flashWait();
    void SPIDataRW(uint8_t* data, int len);
    void readFlashID();
    void initSPI();
    void iceReset();

    uint8_t _inBuffer[512];
    int _size;
    int _addr;
    int _pageOffset;
    int _msTimer;

};


#endif