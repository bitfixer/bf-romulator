#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern void reset6502();
extern void step6502();

extern uint32_t clockticks6502;
uint8_t RAM[65536];
uint8_t overrideRAM[65536];
uint16_t stuckHigh;
uint16_t stuckLow;

uint16_t badAddress;
uint16_t stuckLowOnAddress;

uint8_t read6502(uint16_t address)
{
    if (address < 0x8000)
    {
        address = address | stuckHigh;
    }

    if (address == badAddress)
    {
        printf("returning %X from %X\n", RAM[address] & ~stuckLowOnAddress, address);
        return RAM[address] & ~stuckLowOnAddress;
    }
    

    if (address == 0x7FFF)
    {
        printf("READ %X (%d) -> %X (%d)\n", address, address, RAM[address], RAM[address]);
    }
    return RAM[address];
}

void write6502(uint16_t address, uint8_t value)
{
    if (address < 0x8000)
    {
        address = address | stuckHigh;
    }

    //printf("WRITE %D (%d) <- %X (%d)\n", address, address, value, value);
    if (address >= 0x9000)
    {
        printf("Write RO %X, %X\n", address, value);
        overrideRAM[address] = value;
        return;
    }

    RAM[address] = value;
}

int main(int argc, char** argv)
{
    printf("hey\n");

    stuckHigh = 0;
    stuckLow = 0;
    uint16_t test_start_address = 0x8100;

    badAddress = 0x0304;
    stuckLowOnAddress = 0x01;

    memset(RAM, 0, 65536);
    memset(overrideRAM, 0, 65536);

    FILE* fp = fopen(argv[1], "rb");
    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    printf("size: %d\n", sz);
    fseek(fp, 0, SEEK_SET);
    fread(&RAM[0xF000], 1, sz, fp);
    fclose(fp);

    printf("run program:\n");
    int64_t instructions = 0;

    // set test values in ROM locations
    memset(&RAM[0x9000], 0x90, 4096);
    memset(&RAM[0xA000], 0xA0, 4096);
    memset(&RAM[0xB000], 0xB0, 4096);
    memset(&RAM[0xC000], 0xC0, 4096);
    memset(&RAM[0xD000], 0xD0, 4096);
    memset(&RAM[0xE000], 0xE0, 4096);

    reset6502();

    // simulate address line or byte fault

    while (RAM[0x7FFF] != 0xDD)
    {
        step6502();
        instructions++;
    }

    printf("done, %lld instructions, %d ticks\n", instructions, clockticks6502);
    float seconds = (float)clockticks6502 / 1000000.0;
    printf("%f seconds @ 1MHz\n", seconds);

    uint8_t fault_indicator = RAM[test_start_address + 0x08];
    if (fault_indicator == 0xBB)
    {
        printf("fault detected\n");
        uint8_t fault_page = RAM[test_start_address + 0x04];
        uint8_t fault_byte = RAM[test_start_address + 0x05];

        uint16_t fault_address = (uint16_t)fault_page * 256 + (uint16_t)fault_byte;

        uint8_t expected_value = RAM[test_start_address + 0x06];
        uint8_t read_value = RAM[test_start_address + 0x07];
        printf("address bytes %X %X (%d %d), addr %d, exp %X read %X\n", 
            fault_page, fault_byte, fault_page, fault_byte, fault_address, expected_value, read_value);
    }


    fp = fopen("output.bin", "wb");
    fwrite(RAM, 1, 65535, fp);
    fclose(fp);

    fp = fopen("override.bin", "wb");
    fwrite(overrideRAM, 1, 65535, fp);
    fclose(fp);
}