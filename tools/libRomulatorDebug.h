#ifndef __LIB_ROMULATOR_DEBUG__
#define __LIB_ROMULATOR_DEBUG__

#include <stdint.h>

bool romulatorReadMemory(uint8_t* buffer, int retries);
bool romulatorReadVram(uint8_t* vram, int size, int valid_bytes);
bool romulatorReadVramBlock(uint8* vram);
uint8_t romulatorReadConfig();

#endif