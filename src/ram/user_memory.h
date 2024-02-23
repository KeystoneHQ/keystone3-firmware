#ifndef _USER_MEMORY_H
#define _USER_MEMORY_H

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

void *SramMallocTrack(size_t size, const char *file, int line, const char *func);
void SramFreeTrack(void *p, const char *file, int line, const char *func);
void *SramReallocTrack(void *p, size_t size, const char *file, int line, const char *func);
void *SramMalloc(size_t size);
void SramFree(void *p);

void *ExtMallocTrack(size_t size, const char *file, int line, const char *func);
void ExtFreeTrack(void *p, const char *file, int line, const char *func);
void *ExtMalloc(size_t size);
void ExtFree(void *p);

void *RustMalloc(int32_t size);
void RustFree(void *p);

void PrintHeapInfo(void);

#ifdef COMPILE_SIMULATOR
#include <stdlib.h>
#define SRAM_MALLOC(size)           malloc(size)
#define SRAM_FREE(p)                free(p)
#define SRAM_REALLOC(p, size)       realloc(p)
#else
#include "safe_str_lib.h"
#define SRAM_MALLOC(size)           SramMallocTrack(size, __FILE__, __LINE__, __func__)
#define SRAM_FREE(p)                SramFreeTrack(p, __FILE__, __LINE__, __func__)
#define SRAM_REALLOC(p, size)       SramReallocTrack(p, size, __FILE__, __LINE__, __func__)
#endif

#ifdef COMPILE_SIMULATOR
#define EXT_MALLOC(size)            malloc(size)
#define EXT_FREE(p)                 free(p)
#else
#define EXT_MALLOC(size)            ExtMallocTrack(size, __FILE__, __LINE__, __func__)
#define EXT_FREE(p)                 ExtFreeTrack(p, __FILE__, __LINE__, __func__)
#endif


#endif
