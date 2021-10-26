#ifndef __LIB_ROMULATOR_VRAM_H__
#define __LIB_ROMULATOR_VRAM_H__

#include <stdint.h>

void romulatorVramToBitmap(
    uint8_t* vram,
    uint8_t* characterRom,
    int rows,
    int columns,
    int charWidth,
    int charHeight,
    uint8_t* bitmap);

#endif