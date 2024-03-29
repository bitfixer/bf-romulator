#ifndef __LIB_ROMULATOR_DEBUG__
#define __LIB_ROMULATOR_DEBUG__

#include <stdint.h>

void romulatorSetInput();
void romulatorInitDebug();
void romulatorClose();

void romulatorReset();
void romulatorResetDevice();

bool romulatorReadMemoryToFile();
bool romulatorWriteMemoryFromFile();
bool romulatorReadVram(uint8_t* vram, int size, int valid_bytes, int retries);
uint8_t romulatorReadConfig();
void romulatorWriteConfig(int config);
void romulatorHaltCpu();
void romulatorStartCpu();

void romulatorStartReadMemory();
void romulatorReadMemoryBlock(uint8_t* buf, int size);
uint32_t romulatorReadMemoryCRC(uint8_t* buf);
bool romulatorChangeConfiguration(int configSetting);

void crc32(const void *data, int n_bytes, uint32_t* crc);

#endif