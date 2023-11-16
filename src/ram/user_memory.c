/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: On-chip SRAM and external PSRAM heap memory management.
 * Author: leon sun
 * Create: 2022-11-8
 ************************************************************************************************/

#include "user_memory.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "psram_heap_4.h"
#include "assert.h"

static uint32_t g_sramHeapCount = 0;
static uint32_t g_extHeapCount = 0;
static bool g_isRustMallocPsram = false;

void *SramMalloc(uint32_t size, const char *file, int line, const char *func)
{
    void *p = pvPortMalloc((uint32_t) size);
    // printf("malloc:%s %s %d 0x%X %d\r\n", file, func, line, p, size);
    // ASSERT(p != NULL);
    if (xPortGetFreeHeapSize() < 10000) {
        printf("Sram FreeHeapSize = %d\n", xPortGetFreeHeapSize());
    }
    g_sramHeapCount++;
    return p;
}


void SramFree(void *p, const char *file, int line, const char *func)
{
    // printf("free:%s %s %d 0x%X\r\n", file, func, line, p);
    if (p != NULL) {
        vPortFree(p);
        g_sramHeapCount--;
    }
}


void *SramRealloc(void *p, uint32_t size, const char *file, int line, const char *func)
{
    void *dest;
    dest = pvPortMalloc(size);
    ASSERT(dest != NULL);
    memcpy(dest, p, size);      //todo : only copy useful data.
    vPortFree(p);
    return dest;
}


void *ExtMalloc(uint32_t size, const char *file, int line, const char *func)
{
    void *p = PsramMalloc((uint32_t) size);
    ASSERT(p != NULL);
    if (PsramGetFreeHeapSize() < 10000) {
        printf("Psram FreeHeapSize = %d\n", PsramGetFreeHeapSize());
    }
    g_extHeapCount++;
    return p;
}


void ExtFree(void *p, const char *file, int line, const char *func)
{
    if (p != NULL) {
        PsramFree(p);
        g_extHeapCount--;
    }
}


void SetRustMallocPsram(bool isPsram)
{
    g_isRustMallocPsram = isPsram;
}


void *RustMalloc(int32_t size)
{
    if (g_isRustMallocPsram) {
        return ExtMalloc(size, __FILE__, __LINE__, __func__);
    }
    return SramMalloc(size, __FILE__, __LINE__, __func__);
}


void RustFree(void *p)
{
    if (p >= PSRAM_BASE) {
        ExtFree(p, __FILE__, __LINE__, __func__);
    } else {
        SramFree(p, __FILE__, __LINE__, __func__);
    }
}


void PrintHeapInfo(void)
{
    printf("sram heap info:\r\n");
    printf("g_sramHeapCount = %d\n", g_sramHeapCount);
    printf("TotalHeapSize = %d\n", configTOTAL_HEAP_SIZE);                    // 堆总空间
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());                    // 剩余堆空间
    printf("MinEverFreeHeapSize = %d\n", xPortGetMinimumEverFreeHeapSize());  // 最小的未被分配的堆空间
    printf("\r\next heap info:\r\n");
    printf("g_extHeapCount = %d\n", g_extHeapCount);
    printf("TotalHeapSize = %d\n", PsramGetTotalSize());                    // 堆总空间
    printf("FreeHeapSize = %d\n", PsramGetFreeHeapSize());                    // 剩余堆空间
    printf("MinEverFreeHeapSize = %d\n", PsramGetMinimumEverFreeHeapSize());  // 最小的未被分配的堆空间
}

