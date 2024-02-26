/* header */
#include <stdio.h>
#include <string.h>
#include "define.h"
#include "drv_bpk.h"
#include "safe_mem_lib.h"

/* macro */

/* struct */

/* enum */

/* function declaration */

ErrorStatus SetBpkValue(uint32_t *data, uint32_t len, uint32_t offset)
{
    while (BPK_IsReady() == RESET);
    return BPK_WriteKey(data, len, offset);
}

ErrorStatus ClearBpkValue(uint32_t offset)
{
    while (BPK_IsReady() == RESET);
    uint32_t data[BPK_KEY_LENGTH] = {0};
    memset_s(data, sizeof(data), 0x0, sizeof(data));
    return SetBpkValue(data, BPK_KEY_LENGTH, offset);
}

ErrorStatus GetBpkValue(uint32_t *data, uint32_t len, uint32_t offset)
{
    while (BPK_IsReady() == RESET);
    return BPK_ReadKey(data, len, offset);
}

void PrintBpkValue(uint32_t offset)
{
    uint32_t data[BPK_KEY_LENGTH] = {0};
    ErrorStatus ret = GetBpkValue(data, BPK_KEY_LENGTH, offset);
    if (ret == ERROR) {
        printf("get value failed: %d\n", offset);
        return;
    }
    for (int i = 0; i < BPK_KEY_LENGTH; i++) {
        printf("%08x ", data[i]);
        if (3 == i % 4) {
            printf("\n");
        }
    }
}