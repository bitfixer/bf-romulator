#ifndef __LIB_ROMULATOR_PROGRAMMER_H__
#define __LIB_ROMULATOR_PROGRAMMER_H__

#include <stdint.h>
#include <LittleFS.h>

class RomulatorProgrammer 
{
public:
    RomulatorProgrammer() 
    : _size(0)
    , _addr(0)
    , _pageOffset(0)
    , _msTimer(0)
    , _programmingFromFile(false)
    {};

    ~RomulatorProgrammer() {};

    void reset();
    void beginProgrammingFromFile(char* filename);
    bool updateProgrammingFromFile();
    void beginProgramming(int totalSize);
    bool programBlock(uint8_t* block, int blockSize);
    int getProgrammingPercentage();
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
    bool _programmingFromFile;
    File _fp;
};


#endif