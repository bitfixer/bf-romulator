#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

long getSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return sz;
}

int main(int argc, char** argv)
{
    int opt;
    int setting = 0;
    const char* memorySet = "tools/default_memory_set.csv";
    const char* romDir = "roms/";
    char* binFileName = NULL;

    while ((opt = getopt(argc, argv, "s:b:")) != -1)
    {
        switch (opt)
        {
            case 's':
                setting = atoi(optarg);
                break;
            case 'b':
                binFileName = new char[strlen(optarg)];
                strcpy(binFileName, optarg);
                break;
        }
    }

    printf("setting: %d\n", setting);
    FILE* fpMemorySet = fopen(memorySet, "rb");

    if (!fpMemorySet)
    {
        printf("could not open %s\n", memorySet);
        return 1;
    }

    printf("binFile: %s\n", binFileName);
    FILE* fpBin = fopen(binFileName, "rb");

    long binFileSize = getSize(fpBin);
    uint8_t* binContents = new uint8_t[binFileSize];
    fread(binContents, 1, binFileSize, fpBin);
    fclose(fpBin);

    /*
    // find entries in the csv file
    char* csvLine = new char[256];
    size_t size = 256;
    int len = getline(csvLine, &size, fpMemorySet);
    while (len > 0)
    */
    char line[256];
    while (fgets(line, sizeof(line), fpMemorySet))
    {
        char* token = strtok(line, ",");

        // first field is set index
        int index = atoi(token);
        if (index == setting)
        {
            char romFileName[256];
            uint16_t romAddress;
            token = strtok(NULL, ",");
            
            sprintf(romFileName, "%s%s", romDir, token);
            token = strtok(NULL, ",");

            sscanf(token, "0x%X", &romAddress);
            printf("%s, %X\n", romFileName, romAddress);

            printf("comparing..\n");
            // now open file and compare to binary dump
            FILE* fpRom = fopen(romFileName, "rb");
            long romLen = getSize(fpRom);

            uint8_t byte;
            bool compare_ok = true;
            for (int i = 0; i < romLen; i++)
            {
                // compare byte with byte in dump
                fread(&byte, 1, 1, fpRom);
                int address = romAddress + i;
                if (byte != binContents[address])
                {
                    printf("mismatch at %X: ROM %X BIN %X\n", address, byte, binContents[address]);
                    compare_ok = false;
                }
            }
            if (compare_ok)
            {
                printf("%s compares ok\n", romFileName);
            }
        }
    }

    return 0;
}