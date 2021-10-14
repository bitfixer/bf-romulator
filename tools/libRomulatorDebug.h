#ifndef __LIB_ROMULATOR_DEBUG__
#define __LIB_ROMULATOR_DEBUG__

#include <stdint.h>

bool romulatorReadMemory(uint8_t* buffer, int retries);
void romulatorWriteMemory(uint8_t* send_buffer, bool verify);
bool romulatorReadVram(uint8_t* vram, int size, int valid_bytes, int retries);
bool romulatorReadVramBlock(uint8_t* vram);
uint8_t romulatorReadConfig();
void romulatorHaltCpu();
void romulatorStartCpu();
void crc32(const void *data, int n_bytes, uint32_t* crc);


#endif