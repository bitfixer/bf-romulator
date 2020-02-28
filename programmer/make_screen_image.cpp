#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

// make_screen_image.cpp
// render a PET screen memory dump into an image,
// using a specified character rom

long getSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return sz;
}

void write_ppm_header(int width, int height)
{
    printf("P6\n");
    printf("%d %d\n", width, height);
    printf("255\n");
}

void write_pixel(uint8_t r, uint8_t g, uint8_t b)
{
    //printf("%d %d %d ", r, g, b);
    fwrite(&r, 1, 1, stdout);
    fwrite(&g, 1, 1, stdout);
    fwrite(&b, 1, 1, stdout);
}

void draw_bitmap(uint8_t* bitmap, int image_width, int image_height)
{
    write_ppm_header(image_width, image_height);
    for (int y = 0; y < image_height; y++)
    {
        for (int x = 0; x < image_width; x++)
        {
            int pixel_index = (y * image_width) + x;
            if (bitmap[pixel_index] == 0)
            {
                write_pixel(0, 0, 0);
            }
            else
            {
                write_pixel(255, 255, 255);
            }
        }
    }
}

void draw_character(uint8_t character,
                    int x, 
                    int y, 
                    uint8_t* bitmap, 
                    uint8_t* character_rom, 
                    int image_width)
{

    // get bytes from character roms
    for (int yy = 0; yy < 8; yy++)
    {
        int byte_index = character*8 + yy;
        uint8_t byte = character_rom[byte_index];
        for (int xx = 0; xx < 8; xx++)
        {
            int bit_index = 7 - xx;
            int pixel_index = ((y + yy) * image_width) + (x + xx);

            if ((byte & (1 << bit_index)) == 0)
            {
                //write_pixel(0,0,0);
                bitmap[pixel_index] = 0;
            }
            else
            {
                //write_pixel(255, 255, 255);
                bitmap[pixel_index] = 1;
            }
        }
    }
}

int main(int argc, char** argv) {

    int opt;
    char romname[1024];
    uint8_t* memory;
    memset(romname, 0, 1024);
    int screen_offset = 0x8000;
    int memory_size = 65536;
    int columns = 80;
    int rows = 25;

    while ((opt = getopt(argc, argv, "r:o:m:c:")) != -1)
    {
        switch (opt)
        {
            case 'r':
                strcpy(romname, optarg);
                break;
            case 'o':
                screen_offset = atoi(optarg);
                break;
            case 'm':
                memory_size = atoi(optarg);
                break;
            case 'c':
                columns = atoi(optarg);
                break;
            default:
                break;
        }
    }

    memory = new uint8_t[memory_size];
    memset(memory, 0, memory_size);
    fprintf(stderr, "rom: %s, offset %d\n", romname, screen_offset);

    // read entire rom
    FILE* fp = fopen(romname, "rb");
    long sz = getSize(fp);

    uint8_t* rom = new uint8_t[sz];
    fread(rom, 1, sz, fp);
    fclose(fp);

    // read memory map
    fread(memory, 1, memory_size, stdin);

    int char_width = 8;
    int char_height = 8;
    int image_width = columns * char_width;
    int image_height = rows * char_height;

    uint8_t* bitmap = new uint8_t[image_width * image_height];
    int char_index = 0;
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < columns; col++)
        {
            int x = col * 8;
            int y = row * 8;
            uint8_t character = memory[screen_offset + char_index++];
            draw_character(character, x, y, bitmap, rom, image_width);
        }
    }
    draw_bitmap(bitmap, image_width, image_height);

    delete[] rom;
    delete[] memory;
    delete[] bitmap;
}