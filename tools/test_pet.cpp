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
    while (!rundone)
    {
        int start = millis();
        printf("starting time %d\n", start);
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
        if (memory[0xE809] == 0xDD)
        {
            rundone = true;
        }
    }

    printf("Done.\n");

    return 0;
}