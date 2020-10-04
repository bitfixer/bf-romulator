#include <stdio.h>
#include <wiringPi.h>
#include <iostream.h>

int main(int argc, char** argv)
{
    printf("***ROMulator test program for Commodore PET***\n");
    printf("Place ROMulator in 6502 socket on PET.\nMake sure the notch is facing the right way!\n");
    printf("Connect to the Raspberry PI with the programming jumper OFF.\n");
    printf("Then switch on the PET.\n");
    printf("Press enter to continue...");
    getchar();

    int start = millis();
    int interval = 5000;
    printf("starting time %d\n", start);

    int now = millis();
    while (now < start + interval)
    {
        delay(1000);
        now = millis();
    }

    system("bin/console -r > out.bin");
    printf("Done.\n");

    return 0;
}