#include "libRomulatorVram.h"

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
                bitmap[pixel_index] = 0;
            }
            else
            {
                bitmap[pixel_index] = 1;
            }
        }
    }
}

void romulatorVramToBitmap(
    uint8_t* vram,
    uint8_t* characterRom,
    int rows,
    int columns,
    int charWidth,
    int charHeight,
    uint8_t* bitmap)
{
    int imageWidth = columns * charWidth;
    int imageHeight = rows * charHeight;

    int char_index = 0;
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < columns; col++)
        {
            int x = col * 8;
            int y = row * 8;
            uint8_t character = vram[char_index++];
            draw_character(character, x, y, bitmap, characterRom, imageWidth);
        }
    }
}