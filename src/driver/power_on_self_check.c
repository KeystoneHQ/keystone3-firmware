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
#include "draw_on_lcd.h"
#include "account_manager.h"

#define GD25_FLASH_ID               (0xC84018)
#define PY25_FLASH_ID               (0x852018)
#define SRAM_TEST_LEN               (1024)
#define SRAM_TEST_CHAR              (0xAC)

LV_FONT_DECLARE(openSans_20);
LV_FONT_DECLARE(openSans_24);

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
    uint32_t wipeFlag = 0xFFFFFFFF;
    int32_t readSize = Gd25FlashReadBuffer(SPI_FLASH_ADDR_PROTECT_PARAM, (uint8_t *)&wipeFlag, sizeof(wipeFlag));
    assert(readSize == sizeof(wipeFlag));
    if (wipeFlag == DEVICE_WIPE_FLAG_MAGIC_NUM || AccountManagerIsNeedReset()) {
        DrawStringOnLcd(190, 408, "Loading...", 0xFFFF, &openSans_24);
        uint32_t c = 0x666666;
        DrawStringOnLcd(170, 456, "About 1 minute", (uint16_t)(((c & 0xF80000) >> 16) | ((c & 0xFC00) >> 13) | ((c & 0x1C00) << 3) | ((c & 0xF8) << 5)), &openSans_20);
        FpWipeManageInfo();
        ErasePublicInfo();
        DestroyAccount(0);
        DestroyAccount(1);
        DestroyAccount(2);
        for (uint32_t addr = 0; addr < GD25QXX_FLASH_SIZE; addr += 1024 * 64) {
            Gd25FlashBlockErase(addr);
            printf("flash erase address: %#x\n", addr);
        }
        NVIC_SystemReset();
    }
}