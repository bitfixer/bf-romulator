#include <stdio.h>
#include <string.h>
#include <stdint.h>

unsigned char test_page(
    unsigned char* page_address, 
    unsigned char* page_byte, 
    unsigned char* expected_byte, 
    unsigned char* read_byte)
{
    unsigned char flag_value;
    unsigned char inv_flag_value;
    unsigned char start_position;
    unsigned char interval;
    unsigned char pagedone;
    unsigned char byte;

    flag_value = 0xFF;
    inv_flag_value = 0x00;
    start_position = 3;
    pagedone = 0;
    while (pagedone == 0)
    {
        // first fill the page with the flag value
        memset(page_address, flag_value, 256);
        // then write every third byte
        interval = start_position;
        byte = 0;
        while(1)
        {
            interval--;
            if (interval == 0)
            {
                page_address[byte] = inv_flag_value;
                interval = 3;
            }

            if (++byte == 0)
            {
                break;
            }
        }

        // now compare the page
        interval = start_position;
        byte = 0;
        while(1)
        {
            interval--;
            if (interval == 0)
            {
                if (page_address[byte] != inv_flag_value)
                {
                    *page_byte = byte;
                    *expected_byte = inv_flag_value;
                    *read_byte = page_address[byte];
                    return 0;
                }
                interval = 3;
            }
            else
            {
                if (page_address[byte] != flag_value)
                {
                    *page_byte = byte;
                    *expected_byte = flag_value;
                    *read_byte = page_address[byte];
                    return 0;
                }
            }

            if (++byte == 0)
            {
                break;
            }
        }

        // change flag position
        start_position--;
        if (start_position == 0)
        {
            start_position = 3;
            if (flag_value == 0x00)
            {
                // done
                pagedone = 1;
            }
            else
            {
                flag_value = 0x00;
                inv_flag_value = 0xFF;
            }
        }
    }
    return 1;

}

void read_rom_page(unsigned char* address, uint16_t* checksum)
{
    unsigned char* read_addr = address;
    unsigned char byte;
    unsigned char index = 0;
    uint16_t checksum_val = 0;
    while (1)
    {
        byte = *read_addr;
        *read_addr = byte;

        checksum_val = byte;
        read_addr++;
        index++;

        byte = *read_addr;
        *read_addr = byte;

        checksum_val += (uint16_t)byte << 8;
        read_addr++;
        index++;

        *checksum += checksum_val;

        if (index == 0)
        {
            break;
        }
    }
}

void testPrintf(char* textptr, const char* format, ...)
{
    int i;
    static char buffer[16];
    va_list arg;
    va_start(arg, format);
    vsprintf(buffer, format, arg);

    // copy memory to screen
    memcpy(textptr, buffer, strlen(buffer));
}

void main()
{
    unsigned char* screen = (unsigned char*)0x8000;
    unsigned char* tmp = (unsigned char*)0x7FFF;
    unsigned char* start_address = (unsigned char*)0x0200;
    unsigned char* end_address = (unsigned char*)0x8000;
    unsigned char* text_mode = (unsigned char*)53272;
    unsigned char* test_address;
    unsigned char* textptr;
    unsigned char page_byte;
    unsigned char expected_byte;
    unsigned char read_byte;
    uint16_t checksum;

    // video ram test
    textptr = screen;
    *text_mode = 23;

    for (test_address = (unsigned char*)0x8000; test_address < (unsigned char*)0x9000; test_address += 0x0100)
    {
        if (test_page(test_address, &page_byte, &expected_byte, &read_byte) == 0)
        {
            test_address += page_byte;
            break;
        }
    }

    // video ram test done
    // clear screen
    memset(screen, 0x20, 1024);
    if (test_address == (unsigned char*)0x9000)
    {
        testPrintf(textptr, "VRAM OK");
    }
    else
    {
        testPrintf(textptr, "VRAM %04X", test_address);
        textptr += 40;
        testPrintf(textptr, "EX %02X RE %02X", expected_byte, read_byte);
    }
    textptr += 40;

    testPrintf(textptr, "RAM TEST");
    textptr += 40;

    for (test_address = start_address; test_address < end_address; test_address += 0x0100)
    {
        testPrintf(textptr, "%04X", test_address);
        if (test_page(test_address, &page_byte, &expected_byte, &read_byte) == 0)
        {
            test_address += page_byte;
            testPrintf(textptr, "%04X FAULT", test_address);
            textptr += 40;
            testPrintf(textptr, "EX %02X RE %02X", expected_byte, read_byte);
            break;
        }
    }

    textptr += 40;
    testPrintf(textptr, "ROM READ");
    textptr += 40;

    checksum = 0;
    page_byte = 0x90;
    read_byte = 0;
    for (test_address = (unsigned char*)0x9000; test_address < (unsigned char*)0xE800; test_address += 0x0100)
    {
        //testPrintf(textptr, "%04X", test_address);
        read_rom_page(test_address, &checksum);
        read_byte++;
        if (read_byte == 8)
        {
            read_byte = 0;
            testPrintf(textptr, "%02X %04X", page_byte, checksum);
            checksum = 0;
            page_byte += 8;
            textptr += 40;
        }
    }

    textptr += 40;
    testPrintf(textptr, "DONE");

    *tmp = 0xDD;
    while (1) {}
}