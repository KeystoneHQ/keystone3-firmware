#include "log_print.h"
#include "stdio.h"
#include "librust_c.h"

#ifdef RUST_MEMORY_DEBUG
#include "user_memory.h"
#include "assert.h"
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
    child = (RustMemoryNode_t *)ExtMalloc(sizeof(RustMemoryNode_t), __FILE__, __LINE__, __func__);
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
    if(current == NULL) {
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
    ExtFree(current, __FILE__, __LINE__, __func__);
}

void RustMemoryNode_print()
{
    RustMemoryNode_t *current = rustMemoryListHead;
    char memBuf[128] = {0};
    while (current != NULL) {
        snprintf(memBuf, sizeof(memBuf), "Rust Memory Usage: address: 0x%x, size: %d\n", current -> p, current -> size);
        WriteDebugToSdcard(memBuf, strlen(memBuf));
        printf("Rust Memory Usage: address: 0x%x, size: %d, possible string value: %s\r\n", current -> p, current -> size, current -> p);
        current = current -> next;
    }
}
#endif

void PrintRustMemoryStatus()
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
    printf("message from rust: %s\r\n", str);
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

LV_FONT_DECLARE(openSans_20);

void LogRustPanic(char* panic_info)
{
    PrintOnLcd(&openSans_20, 0xFFFF, "Rust Panic: %s\r\n", panic_info);
    PrintErrorInfoOnLcd();
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
}

#endif

