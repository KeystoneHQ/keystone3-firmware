#include <stdio.h>
#include <string.h>
#include "mhscpu.h"
#include "err_code.h"
#include "drv_gd25qxx.h"
#include "flash_address.h"
#include "device_setting.h"
#include "user_memory.h"
#include "assert.h"
#include "fingerprint_process.h"
#include "keystore.h"

#define GD25_FLASH_ID               (0xC84018)
#define PY25_FLASH_ID               (0x852018)
#define SRAM_TEST_LEN               (1024)
#define SRAM_TEST_CHAR              (0xAC)

void PowerOnSelfCheck(void)
{
    int ret = SUCCESS_CODE;
    uint32_t flashId = 0;

    // flash read id
    flashId = Gd25FlashReadID();
    printf("spi flash id = %#x\n", flashId);
    if (flashId != GD25_FLASH_ID && flashId != PY25_FLASH_ID) {
        ret = ERR_GD25_BAD_PARAM;
    }
    assert(ret == SUCCESS_CODE);

    // psram
    uint8_t *data = EXT_MALLOC(SRAM_TEST_LEN);
    memset(data, SRAM_TEST_CHAR, SRAM_TEST_LEN);
    for (int i = 0; i < SRAM_TEST_LEN; i++) {
        if (data[i] != SRAM_TEST_CHAR) {
            ret = ERR_PSRAM_OPERATE_FAILED;
            break;
        }
    }
    EXT_FREE(data);
    assert(ret == SUCCESS_CODE);

    // check after wipe device
    uint32_t wipeFlag = 0;
    Gd25FlashReadBuffer(SPI_FLASH_ADDR_PROTECT_PARAM, (uint8_t *)&wipeFlag, sizeof(wipeFlag));
    if (wipeFlag == DEVICE_WIPE_FLAG_MAGIC_NUM) {
        FpWipeManageInfo();
        DestroyAccount(0);
        DestroyAccount(1);
        DestroyAccount(2);
        ErasePublicInfo();
        Gd25FlashChipErase();
        NVIC_SystemReset();
    }
}