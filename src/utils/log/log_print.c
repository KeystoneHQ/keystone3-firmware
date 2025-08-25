#include "log_print.h"
#include "stdio.h"
#include "librust_c.h"

#ifdef RUST_MEMORY_DEBUG
#include "user_memory.h"
#include "assert.h"
#include "string.h"
#include "safe_str_lib.h"

#define MEM_DEBUG_BUF_SIZE 128

struct __RustMemoryNode {
    void *p;
    uint32_t size;
    struct __RustMemoryNode *next;
    struct __RustMemoryNode *prev;
};
typedef struct __RustMemoryNode RustMemoryNode_t;

void WriteDebugToSdcard(char *buf, uint16_t len);

RustMemoryNode_t *rustMemoryListHead = NULL;

void RustMemoryNode_add(void *p, uint32_t size)
{
    // printf("RustMalloc 0x%x, %d\r\n", p, size);
    RustMemoryNode_t *child = rustMemoryListHead;
    RustMemoryNode_t *parent = NULL;
    while (child != NULL) {
        parent = child;
        child = child->next;
    }
    child = (RustMemoryNode_t *)ExtMalloc(sizeof(RustMemoryNode_t));
    child -> p = p;
    child -> size = size;
    child -> prev = parent;
    child -> next = NULL;
    if (parent != NULL) {
        parent -> next = child;
    }
    if (rustMemoryListHead == NULL) {
        rustMemoryListHead = child;
    }
}

void RustMemoryNode_remove(void *p)
{
    RustMemoryNode_t *current = rustMemoryListHead;
    if (current == NULL) {
        //empty list
        return;
    }
    while (current != NULL && current -> p != p) {
        current = current -> next;
    }
    //current must not be NULL, or the memory have already been free.
    // ASSERT(current != NULL);
    if (current == NULL) {
        printf("pointer not found: %p\r\n", p);
        return;
    }
    // printf("RustFree 0x%x, %d\r\n", current->p, current -> size);
    current -> next -> prev = current -> prev;
    if (current -> prev != NULL) {
        current -> prev -> next = current -> next;
    }
    if (current -> prev == NULL) {
        rustMemoryListHead = current -> next;
    }
    ExtFree(current);
}

void RustMemoryNode_print()
{
    RustMemoryNode_t *current = rustMemoryListHead;
    char memBuf[MEM_DEBUG_BUF_SIZE] = {0};
    while (current != NULL) {
        snprintf(memBuf, MEM_DEBUG_BUF_SIZE, "Rust Memory Usage: address: 0x%x, size: %d\n", current -> p, current -> size);
        WriteDebugToSdcard(memBuf, strnlen_s(memBuf, MEM_DEBUG_BUF_SIZE));
        printf("Rust Memory Usage: address: 0x%x, size: %d\r\n", current->p, current->size);
        if (sizeof(current->p[0]) == 1) {
            if (((char *)current->p)[current->size - 1] == '\0') {
                snprintf(memBuf, sizeof(memBuf), "Rust Memory Possible value: %s\r\n", current->p);
                WriteDebugToSdcard(memBuf, strnlen_s(memBuf, MEM_DEBUG_BUF_SIZE));
                printf("Rust Memory Possible value: %s\r\n", current->p);
            }
        }
        current = current->next;
    }
}
#endif

void PrintRustMemoryStatus(
)
{
#ifdef RUST_MEMORY_DEBUG
    printf("Rust Memory Status: \r\n");
    RustMemoryNode_print();
#else
    printf("Open RUST_DEBUG_MEMORY to print rust memory status\r\n");
#endif

}

void PrintString(char *str)
{
#ifndef BUILD_PRODUCTION
    printf("message from rust: %s\r\n", str);
#endif
#ifndef COMPILE_SIMULATOR
    free_rust_value(str);
#endif
}

void PrintArray(const char *name, const uint8_t *data, uint16_t length)
{
    printf("%s,length=%d\r\n", name, length);
    for (uint32_t i = 0; i < length; i++) {
        if (i % 32 == 0 && i != 0) {
            printf("\r\n");
        }
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}

void PrintU16Array(const char *name, const uint16_t *data, uint16_t length)
{
    printf("%s,length=%d\r\n", name, length);
    for (uint32_t i = 0; i < length; i++) {
        if (i % 16 == 0 && i != 0) {
            printf("\r\n");
        }
        printf("%04X ", data[i]);
    }
    printf("\r\n");
}

void PrintU32Array(const char *name, const uint32_t *data, uint16_t length)
{
    printf("%s,length=%d\r\n", name, length);
    for (uint32_t i = 0; i < length; i++) {
        if (i % 8 == 0 && i != 0) {
            printf("\r\n");
        }
        printf("%8X ", data[i]);
    }
    printf("\r\n");
}

void LogRustMalloc(void *p, uint32_t size)
{
#ifdef RUST_MEMORY_DEBUG
    RustMemoryNode_add(p, size);
#endif
}

void LogRustFree(void *p)
{
#ifdef RUST_MEMORY_DEBUG
    RustMemoryNode_remove(p);
#endif
}

#ifdef COMPILE_SIMULATOR

void LogRustPanic(char* panic_info)
{
    printf("Rust Panic: %s\r\n", panic_info);
}

#else

#include "draw_on_lcd.h"
#include "presetting.h"
#include "version.h"
#include "hardware_version.h"

LV_FONT_DECLARE(openSans_20);

void LogRustPanic(char* panic_info)
{
    PrintOnLcd(&openSans_20, 0xFFFF, "The error was caused by a failed data request.\nYour assets remain safe.\n");
    PrintErrorInfoOnLcd();
    uint32_t c = 0x666666;
    uint16_t color = (uint16_t)(((c & 0xF80000) >> 16) | ((c & 0xFC00) >> 13) | ((c & 0x1C00) << 3) | ((c & 0xF8) << 5));
    PrintOnLcd(&openSans_20, color, "Rust Panic: %s\r\n", panic_info);
    while (1);
}

void PrintErrorInfoOnLcd(void)
{
    char serialNumber[SERIAL_NUMBER_MAX_LEN];
    PrintOnLcd(&openSans_20, 0xFFFF, "Request failed. Restart by long-pressing power\nbutton for 12 secs.\n");
    PrintOnLcd(&openSans_20, 0xFFFF, "If issue persists, please contact\n");
    PrintOnLcd(&openSans_20, 0x1927, "support@Keyst.one\n");
    GetSerialNumber(serialNumber);
    PrintOnLcd(&openSans_20, 0xFFFF, "Serial No.%s\n", serialNumber);
    PrintOnLcd(&openSans_20, 0xFFFF, "Software:%s\n", GetSoftwareVersionString());
    PrintOnLcd(&openSans_20, 0xFFFF, "Hardware:%s\n", GetHardwareVersionString());
}

#endif
