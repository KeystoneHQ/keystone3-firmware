#ifndef _DRV_PSRAM_H
#define _DRV_PSRAM_H


/*
//defined in mhscpu.h
#define MHSCPU_PSRAM_BASE                       (0x80000000UL)  //(0x807FC000UL)
#define MHSCPU_PSRAM_END                        (0x9FFFFFFFUL)
#define MHSCPU_PSRAM_SIZE                       (0x00800000UL)  //(0x00004000UL)    8MBytes
*/

#include "stdint.h"
#include "stdbool.h"


void PsramInit(void);
void PsramOpen(void);
void PsramTest(void);
void PsramSelfCheck(void);
uint32_t GetPsramMappedAddr(void);
uint32_t GetPsramTotalSize(void);

#endif
