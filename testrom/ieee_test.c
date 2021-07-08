#include <stdio.h>
#include <string.h>
#include <stdint.h>

void testPrintf(char* textptr, const char* format, ...)
{
    static char buffer[16];
    va_list arg;
    va_start(arg, format);
    vsprintf(buffer, format, arg);

    // copy memory to screen
    memcpy(textptr, buffer, strlen(buffer));
}

void poke(uint16_t addr, uint8_t val)
{
    unsigned char* ptr = (unsigned char*)addr;
    *ptr = val;
}

unsigned char peek(uint16_t addr)
{
    unsigned char* ptr = (unsigned char*)addr;
    return *ptr;
}

unsigned char test_signal(uint16_t addr, uint16_t rdaddr, unsigned char start, unsigned char end, unsigned char msk, unsigned char test)
{
    unsigned char c;
    unsigned char c1;
    unsigned char a;
    
    poke(addr, start);
    c = peek(rdaddr);
    poke(addr, end);
    c1 = peek(rdaddr);

    a = c & msk;
    if (a == test)
    {
        return 0; // failed
    }

    a = c1 & msk;
    if (a == test)
    {
        return 1; // success
    }

    return 0; // failed
}

unsigned char test_ndac()
{
    unsigned char success;
    success = test_signal(0xE821, 0xE840, 52, 60, 1, 1);
    poke(0xE821,60);
    return success;
}

unsigned char test_dav()
{
    unsigned char success;
    success = test_signal(0xE823, 0xE840, 52, 60, 128, 128);
    poke(0xE821,60);
    return success;
}

unsigned char test_nrfd()
{
    unsigned char success;
    success = test_signal(0xE840, 0xE840, 255, 253, 64, 0);
    poke(0xE840,255);
    return success;
}

unsigned char test_atn()
{
    unsigned char msk;
    unsigned char a;
    unsigned char c;
    unsigned char c1;

    poke(0xE840,251);
    c = peek(0xE821);
    c1 = peek(0xE820);
    poke(0xE840,255);
    c1 = peek(0xE821);

    msk = 128;
    a = c & msk;
    if (a == 0)
    {
        return 0; // failed
    }

    a = c1 & msk;
    if (a == 0)
    {
        return 1; // success
    }

    return 0;
}

unsigned char test_eoi()
{
    return test_signal(0xE811,0xE810,52,60,64,64);
}

void main()
{
    unsigned char* screen = (unsigned char*)0x8000;
    unsigned char* textptr;
    unsigned char* ptr = (unsigned char*)0xE822;
    unsigned char* rptr = (unsigned char*)0xE820;
    unsigned char* p2 = (unsigned char*)0xE821;
    unsigned char c;
    unsigned char c1;
    unsigned char c2;
    unsigned char success;

    textptr = screen;

    memset(screen, 0x20, 1024);
    testPrintf(textptr, "TEST IEEE");
    textptr += 40;
    *ptr = 0;

    c = *rptr;
    c1 = c & 255;

    *ptr = 255;

    //c2 = ~(*rptr ) & 255;
    c2 = *rptr;
    testPrintf(textptr, "C1 %X C2 %X", c1, c2);
    textptr += 40;

    success = test_ndac();
    if (success == 0)
    {
        testPrintf(textptr, "NDAC OK");
    }
    else
    {
        testPrintf(textptr, "NDAC BAD");
    }
    textptr += 40;

    success = test_dav();
    if (success == 0)
    {
        testPrintf(textptr, "DAV OK");
    }
    else
    {
        testPrintf(textptr, "DAV BAD");
    }
    textptr += 40;

    success = test_nrfd();
    if (success == 0)
    {
        testPrintf(textptr, "NRFD OK");
    }
    else
    {
        testPrintf(textptr, "NRFD BAD");
    }
    textptr += 40;

    success = test_atn();
    if (success == 0)
    {
        testPrintf(textptr, "ATN OK");
    }
    else
    {
        testPrintf(textptr, "ATN BAD");
    }
    textptr += 40;

    success = test_eoi();
    if (success == 0)
    {
        testPrintf(textptr, "EOI OK");
    }
    else
    {
        testPrintf(textptr, "EOI BAD");
    }
    textptr += 40;

    testPrintf(textptr, "END OF TEST");

    while (1) {}
}