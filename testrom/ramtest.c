#include <stdio.h>
#include <string.h>
#include <stdint.h>

unsigned char test_page(unsigned char* page_address, unsigned char* page_byte)
{
    unsigned char flag_value;
    unsigned char inv_flag_value;
    unsigned char start_position;
    unsigned char interval;
    unsigned char done;
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
        done = 0;
        while (done == 0)
        {
            interval--;
            if (interval == 0)
            {
                page_address[byte] = inv_flag_value;
                interval = 3;
            }

            if (++byte == 0)
            {
                done = 1;
            }
        }

        // now compare the page
        interval = start_position;
        byte = 0;
        done = 0;
        while (done == 0)
        {
            interval--;
            if (interval == 0)
            {
                if (page_address[byte] != inv_flag_value)
                {
                    *page_byte = byte;
                    return 0;
                }
                interval = 3;
            }
            else
            {
                if (page_address[byte] != flag_value)
                {
                    *page_byte = byte;
                    return 0;
                }
            }

            if (++byte == 0)
            {
                done = 1;
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

void main()
{
    unsigned char* screen = (unsigned char*)0x8000;
    unsigned char* tmp = (unsigned char*)0xFA;
    unsigned char* start_address = (unsigned char*)0x0200;
    unsigned char* end_address = (unsigned char*)0x0F00;
    unsigned char* test_address;
    unsigned char* textptr;
    unsigned char page_byte;

    // clear screen
    memset(screen, 0x20, 1024);
    textptr = screen;
    sprintf(textptr, "RAM TEST");
    textptr += 40;

    for (test_address = start_address; test_address < end_address; test_address += 0x0100)
    {
        sprintf(textptr, "%04X", test_address);
        if (test_page(test_address, &page_byte) == 0)
        {
            test_address += page_byte;
            sprintf(textptr, "%04X FAULT", test_address);
            break;
        }
    }

    *tmp = 0xDD;
    while (1) {}
}