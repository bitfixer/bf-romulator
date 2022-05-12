#include <stdio.h>
#include <stdlib.h>

long getSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return sz;
}

int main(int argc, char** argv)
{
    char* fname = argv[1];
    FILE* fp = fopen(fname, "rb");
    long sz = getSize(fp);

    char* rom_fname = argv[2];
    FILE* fp_rom = fopen(rom_fname, "rb");
    long sz_rom = getSize(fp_rom);

    char* enable_table_fname = argv[3];
    FILE* fp_et = fopen(enable_table_fname, "rb");
    long sz_et = getSize(fp_et);

    long rom_start = 0x20000;
    long diff = rom_start - sz;
    
    // copy fpga image
    unsigned char* first = (unsigned char*)malloc(sizeof(unsigned char) * sz);
    fread(first, 1, sz, fp);
    fclose(fp);

    unsigned char* rom = (unsigned char*)malloc(sizeof(unsigned char) * sz_rom);
    fread(rom, 1, sz_rom, fp_rom);
    fclose(fp_rom);

    unsigned char* enable_table = (unsigned char*)malloc(sizeof(unsigned char) * sz_et);
    fread(enable_table, 1, sz_et, fp_et);
    fclose(fp_et);

    fprintf(stderr, "fpga size %d, rom size %d\n", sz, sz_rom);

    fwrite(first, 1, sz, stdout);
    unsigned char z = 0;
    for (int i = 0; i < diff; i++)
    {
        fwrite(&z, 1, 1, stdout);
    }

    fwrite(rom, 1, sz_rom, stdout);
    fwrite(enable_table, 1, sz_et, stdout);

    free(first);
    free(rom);
    free(enable_table);

    return 0;
}