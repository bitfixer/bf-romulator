#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    uint8_t val = 0x00;
    for (int i = 0; i < 1024; i++)
    {
        printf("%02X\n", val++);
    }
}