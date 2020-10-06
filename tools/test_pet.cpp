#include <stdio.h>
#include <wiringPi.h>
#include <stdint.h>
#include <iostream>

int main(int argc, char** argv)
{
    printf("***ROMulator test program for Commodore PET***\n");
    printf("Place ROMulator in 6502 socket on PET.\nMake sure the notch is facing the right way!\n");
    printf("Connect to the Raspberry PI with the programming jumper OFF.\n");
    printf("Then switch on the PET.\n");
    printf("Press enter to continue...");
    getchar();

    
    int interval = 5000;
    uint8_t memory[65536];
    bool rundone = false;

    uint16_t test_address_start                      =   0xE800;
    uint16_t zero_page_compare_value                 =   test_address_start;
    uint16_t zero_page_address                       =   test_address_start + 1;
    uint16_t zero_page_mismatch_value                =   test_address_start + 2;
    uint16_t ram_test_address                        =   test_address_start + 3;
    uint16_t ram_test_address_page                   =   test_address_start + 4;
    uint16_t ram_test_compare_value                  =   test_address_start + 5;
    uint16_t ram_test_mismatch_value                 =   test_address_start + 6;
    uint16_t ram_test_mismatch_indicator_address     =   test_address_start + 7;
    uint16_t ram_test_complete_indicator_address     =   test_address_start + 8;
    uint16_t done_indicator_address                  =   test_address_start + 9;

    uint16_t ram_space_start                         =   0x01;
    uint16_t ram_space_end                           =   0x80;

    uint16_t rom_space_start                         =   0x90;
    uint16_t rom_space_end                           =   0xFF;

    uint16_t ram_test_mismatch_marker                =   0xBB;
    uint16_t ram_test_complete_marker                =   0xCC;
    uint16_t done_marker                             =   0xDD;

    while (!rundone)
    {
        int start = millis();
        printf("Running..\n");
        int now = start;
        while (now < start + interval)
        {
            delay(1000);
            now = millis();
        }

        system("bin/console -r > out.bin");

        // check the memory dump
        FILE* fp = fopen("out.bin", "rb");
        fread(memory, 1, 65536, fp);
        fclose(fp);

        // check status
        if (memory[done_indicator_address] == done_marker)
        {
            // byte set indicating the test is complete
            rundone = true;
        }
    }

    // check zero page status
    printf("zero page (0x00 - 0xFF): ");
    if (memory[zero_page_address] == 0xFF && memory[zero_page_compare_value] == memory[zero_page_mismatch_value])
    {
        // zero page success
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED: Address %X expected %X, read %X\n", 
            memory[zero_page_address], memory[zero_page_compare_value], memory[zero_page_mismatch_value]);
        printf("Zero page failed, exiting.\n");
        exit(1);
    }

    printf("ram test (0x%X00 - 0x%X00):", ram_space_start, ram_space_end);
    if (memory[ram_test_complete_indicator_address] == ram_test_complete_marker)
    {
        printf("SUCCESS\n");
    }
    else
    {
        uint8_t last_good_address_page = memory[ram_test_address_page];
        uint8_t last_good_address = memory[ram_test_address];

        uint16_t numbytes = ((uint16_t)last_good_address_page << 8) + (uint16_t)last_good_address;

        printf("%d (%X) bytes succeeded. Failed at byte %d (%X) expected %X, read %X\n", 
            numbytes-1, numbytes-1, numbytes, numbytes, memory[ram_test_compare_value], memory[ram_test_mismatch_value]);
    }

    printf("Done.\n");

    return 0;
}