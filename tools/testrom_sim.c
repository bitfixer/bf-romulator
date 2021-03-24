#include <stdio.h>
#include <stdint.h>

extern void reset6502();
extern void step6502();
uint8_t RAM[65536];

uint8_t read6502(uint16_t address)
{
    return RAM[address];
}

void write6502(uint16_t address, uint8_t value)
{
    RAM[address] = value;
}

int main(int argc, char** argv)
{
    printf("hey\n");

    FILE* fp = fopen("bin/testrom.out", "rb");
    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    printf("size: %d\n", sz);
    fseek(fp, 0, SEEK_SET);
    fread(&RAM[0xFF00], 1, sz, fp);
    fclose(fp);

    for (int i = 0xFF00; i <= 0xFFFF; i++)
    {
        printf("%d: %X\n", i, RAM[i]);
    }

    printf("run program:\n");
    int64_t instructions = 0;

    reset6502();

    while (RAM[0xE809] != 0xDD)
    {
        step6502();
        instructions++;
    }

    printf("done, %lld instructions\n", instructions);

    fp = fopen("output.bin", "wb");
    fwrite(RAM, 1, 65535, fp);
    fclose(fp);
}