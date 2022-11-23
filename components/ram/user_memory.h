/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: On-chip SRAM and external PSRAM heap memory management.
 * Author: leon sun
 * Create: 2022-11-8
 ************************************************************************************************/

#ifndef _USER_MEMORY_H
#define _USER_MEMORY_H

#include "stdint.h"
#include "stdbool.h"

void *SramMalloc(uint32_t size, const char *file, int line, const char *func);
void SramFree(void *p, const char *file, int line, const char *func);
void *SramRealloc(void *p, uint32_t size, const char *file, int line, const char *func);
void *ExtMalloc(uint32_t size, const char *file, int line, const char *func);
void ExtFree(void *p, const char *file, int line, const char *func);

void *RustMalloc(int32_t size);
void RustFree(void *p);

void PrintHeapInfo(void);

#ifdef COMPILE_SIMULATOR
#include <stdlib.h>
#define SRAM_MALLOC(size)           malloc(size)
#define SRAM_FREE(p)                free(p)
#define SRAM_REALLOC(p, size)       realloc(p)
#else
#define SRAM_MALLOC(size)           SramMalloc(size, __FILE__, __LINE__, __func__)
#define SRAM_FREE(p)                SramFree(p, __FILE__, __LINE__, __func__)
#define SRAM_REALLOC(p, size)       SramRealloc(p, size, __FILE__, __LINE__, __func__)
#endif

#ifdef COMPILE_SIMULATOR
#define EXT_MALLOC(size)           malloc(size)
#define EXT_FREE(p)                free(p)
#else
#define EXT_MALLOC(size)            ExtMalloc(size, __FILE__, __LINE__, __func__)
#define EXT_FREE(p)                 ExtFree(p, __FILE__, __LINE__, __func__)
#endif


#endif
