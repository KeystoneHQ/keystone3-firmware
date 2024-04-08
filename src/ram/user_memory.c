#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "psram_heap_4.h"
#include "assert.h"
#include "user_memory.h"

#define SRAM_HEAP_TRACK                 0
#define EXT_HEAP_TRACK                  0

static uint32_t g_sramHeapCount = 0;
static uint32_t g_extHeapCount = 0;

void *SramMallocTrack(size_t size, const char *file, int line, const char *func)
{
    void *p = pvPortMalloc(size);
#if (SRAM_HEAP_TRACK == 1)
    printf("sram malloc:%s %s %d 0x%X %d\n", file, func, line, p, size);
#endif
    ASSERT(p != NULL);
    g_sramHeapCount++;
    return p;
}

void SramFreeTrack(void *p, const char *file, int line, const char *func)
{
#if (SRAM_HEAP_TRACK == 1)
    printf("sram free:%s %s %d 0x%X\n", file, func, line, p);
#endif
    if (p != NULL) {
        vPortFree(p);
        g_sramHeapCount--;
    }
}

void *SramReallocTrack(void *p, size_t size, const char *file, int line, const char *func)
{
    void *dest;
#if (SRAM_HEAP_TRACK == 1)
    printf("sram realloc:%s %s %d 0x%X %d\n", file, func, line, p, size);
#endif
    dest = pvPortMalloc(size);
    ASSERT(dest != NULL);
    memcpy(dest, p, size);
    vPortFree(p);
    return dest;
}

void *SramMalloc(size_t size)
{
    void *p = pvPortMalloc((size_t) size);
    g_sramHeapCount++;
    return p;
}

void SramFree(void *p)
{
    if (p != NULL) {
        vPortFree(p);
        g_sramHeapCount--;
    }
}

void *ExtMallocTrack(size_t size, const char *file, int line, const char *func)
{
#if (EXT_HEAP_TRACK == 1)
    printf("ext malloc:%s %s %d 0x%X %d\n", file, func, line, p, size);
#endif
    void *p = PsramMalloc((size_t) size);
    ASSERT(p != NULL);
    g_extHeapCount++;
    return p;
}

void ExtFreeTrack(void *p, const char *file, int line, const char *func)
{
#if (EXT_HEAP_TRACK == 1)
    printf("ext free:%s %s %d 0x%X\n", file, func, line, p);
#endif
    if (p != NULL) {
        PsramFree(p);
        g_extHeapCount--;
    }
}

void *ExtMalloc(size_t size)
{
    void *p = PsramMalloc(size);
    ASSERT(p != NULL);
    g_extHeapCount++;
    return p;
}

void ExtFree(void *p)
{
    if (p != NULL) {
        PsramFree(p);
        g_extHeapCount--;
    }
}

void *ExtRealloc(void *p, size_t newSize)
{
    if (p == NULL) {
        return ExtMalloc(newSize);
    }

    if (newSize == 0) {
        ExtFree(p);
        return NULL;
    }

    ExtFree(p);
    return ExtMalloc(newSize);
}

void *RustMalloc(int32_t size)
{
    return SramMalloc(size);
}

void RustFree(void *p)
{
    SramFree(p);
}

void PrintHeapInfo(void)
{
    printf("sram heap info:\n");
    printf("g_sramHeapCount = %d\n", g_sramHeapCount);
    printf("TotalHeapSize = %d\n", configTOTAL_HEAP_SIZE);                      // Total heap size
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());                      // Free heap space
    printf("MinEverFreeHeapSize = %d\n", xPortGetMinimumEverFreeHeapSize());    // Minimum amount of unallocated heap space
    printf("\next heap info:\n");
    printf("g_extHeapCount = %d\n", g_extHeapCount);
    printf("TotalHeapSize = %d\n", PsramGetTotalSize());                        // Total heap size
    printf("FreeHeapSize = %d\n", PsramGetFreeHeapSize());                      // Free heap space
    printf("MinEverFreeHeapSize = %d\n", PsramGetMinimumEverFreeHeapSize());    // Minimum amount of unallocated heap space
}
