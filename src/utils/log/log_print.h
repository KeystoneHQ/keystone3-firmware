#ifndef _LOG_PRINT_H
#define _LOG_PRINT_H

#include "stdint.h"
#include "stdbool.h"

#ifdef DEBUG_MEMORY
#define RUST_MEMORY_DEBUG
#endif

void PrintString(char *str);
void PrintArray(const char *name, const uint8_t *data, uint16_t length);
void PrintU16Array(const char *name, const uint16_t *data, uint16_t length);
void PrintU32Array(const char *name, const uint32_t *data, uint16_t length);
void LogRustMalloc(void *p, uint32_t size);
void LogRustFree(void *p);
void PrintRustMemoryStatus();
void PrintErrorInfoOnLcd(void);

#endif
