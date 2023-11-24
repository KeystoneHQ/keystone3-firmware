#include "user_memory.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "psram_heap_4.h"
#include "assert.h"

static uint32_t g_sramHeapCount = 0;
static uint32_t g_extHeapCount = 0;

void *SramMalloc(uint32_t size, const char *file, int line, const char *func)
{
    void *p = pvPortMalloc((uint32_t) size);
    // printf("malloc:%s %s %d 0x%X %d\r\n", file, func, line, p, size);
    // ASSERT(p != NULL);
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


void *RustMalloc(int32_t size)
{
    return SramMalloc(size, __FILE__, __LINE__, __func__);
}


void RustFree(void *p)
{
    SramFree(p, __FILE__, __LINE__, __func__);
}


void PrintHeapInfo(void)
{
    printf("sram heap info:\r\n");
    printf("g_sramHeapCount = %d\n", g_sramHeapCount);
    printf("TotalHeapSize = %d\n", configTOTAL_HEAP_SIZE);                      // Total heap size
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());                      // Free heap space
    printf("MinEverFreeHeapSize = %d\n", xPortGetMinimumEverFreeHeapSize());    // Minimum amount of unallocated heap space
    printf("\r\next heap info:\r\n");
    printf("g_extHeapCount = %d\n", g_extHeapCount);
    printf("TotalHeapSize = %d\n", PsramGetTotalSize());                        // Total heap size
    printf("FreeHeapSize = %d\n", PsramGetFreeHeapSize());                      // Free heap space
    printf("MinEverFreeHeapSize = %d\n", PsramGetMinimumEverFreeHeapSize());    // Minimum amount of unallocated heap space
}

