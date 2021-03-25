#include <stdio.h>
#include <stdint.h>

extern void reset6502();
extern void step6502();
uint8_t RAM[65536];
uint16_t stuckHigh;
uint16_t stuckLow;

uint16_t badAddress;
uint16_t stuckLowOnAddress;

uint8_t read6502(uint16_t address)
{
    if (address < 0x9000)
    {
        address = address | stuckHigh;
    }

    if (address == badAddress)
    {
        return RAM[address] & ~stuckLowOnAddress;
    }
    return RAM[address];
}

void write6502(uint16_t address, uint8_t value)
{
    if (address < 0x9000)
    {
        address = address | stuckHigh;
    }

    RAM[address] = value;
}

int main(int argc, char** argv)
{
    printf("hey\n");

    stuckHigh = 0;
    stuckLow = 0;

    badAddress = 123;
    stuckLowOnAddress = 0x01;

    FILE* fp = fopen("bin/testromv2.out", "rb");
    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    printf("size: %d\n", sz);
    fseek(fp, 0, SEEK_SET);
    fread(&RAM[0xFF00], 1, sz, fp);
    fclose(fp);

    printf("run program:\n");
    int64_t instructions = 0;

    reset6502();

    // simulate address line or byte fault

    while (RAM[0xE809] != 0xDD)
    {
        step6502();
        instructions++;
    }

    printf("done, %lld instructions\n", instructions);

    uint8_t fault_indicator = RAM[0xE808];
    if (fault_indicator == 0xBB)
    {
        printf("fault detected\n");
        uint8_t fault_page = RAM[0xE806];
        uint8_t fault_byte = RAM[0xE807];

        uint8_t expected_value = RAM[0xE804];
        uint8_t read_value = RAM[0xE805];
        printf("address %X %X (%d %d) exp %X read %X\n", fault_page, fault_byte, fault_page, fault_byte, expected_value, read_value);
    }


    fp = fopen("output.bin", "wb");
    fwrite(RAM, 1, 65535, fp);
    fclose(fp);
}