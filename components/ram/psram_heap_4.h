/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: heap_4 on PSRAM.
 * Author: leon sun
 * Create: 2022-11-11
 ************************************************************************************************/

#ifndef _PSRAM_HEAP_H
#define _PSRAM_HEAP_H

//PSRAM 8MB
#define PSRAM_HEAP_SIZE         (0x800000 - 0x1000)

#include <stdlib.h>
#include "stdint.h"

void *PsramMalloc(size_t xWantedSize);
void PsramFree(void *pv);
size_t PsramGetFreeHeapSize(void);
size_t PsramGetMinimumEverFreeHeapSize(void);
size_t PsramGetTotalSize(void);

#endif


