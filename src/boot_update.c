#include "define.h"
#include "mhscpu.h"
#include "user_fatfs.h"
#include "user_memory.h"
#include "draw_on_lcd.h"
#include "sha256.h"
#include "gui_model.h"
#include "cmsis_os2.h"
#include "drv_qspi_flash.h"

LV_FONT_DECLARE(openSans_24);

#define BOOT_ADDR                           (0x01001000)
#define SECTOR_SIZE                         4096
#define BOOT_HEAD_SIZE                      0x104
#define APP_ADDR                            (0x1001000 + 0x80000)   //108 1000
#define APP_CHECK_START_ADDR                (APP_ADDR)
#define APP_END_ADDR                        (0x2000000)
static const uint8_t MAGIC_NUMBER[] = {'m', 'h', '1', '9', '0', '3', 'b', 'o', 'o', 't', 'u', 'p', 'd', 'a', 't', 'e'};

static uint8_t g_fileUnit[4096] = {0};

static uint32_t BinarySearchBootHead(void)
{
    size_t MAGIC_NUMBER_SIZE = sizeof(MAGIC_NUMBER);
    uint8_t *buffer = SRAM_MALLOC(SECTOR_SIZE);
    uint32_t startIndex = (APP_CHECK_START_ADDR - APP_ADDR) / SECTOR_SIZE;
    uint32_t endIndex = (APP_END_ADDR - APP_ADDR) / SECTOR_SIZE;

    for (int i = startIndex + 1; i < endIndex; i++) {
        memcpy_s(buffer, SECTOR_SIZE, (uint32_t *)(APP_ADDR + i * SECTOR_SIZE), SECTOR_SIZE);
        if (memcmp(buffer, MAGIC_NUMBER, MAGIC_NUMBER_SIZE) == 0) {
            printf("find magic number\n");
            return i;
        }
    }
    SRAM_FREE(buffer);
    return -1;
}

static void Delay(uint32_t ms)
{
    uint32_t i, tick, countPerMs;
    countPerMs = SYSCTRL->HCLK_1MS_VAL / 4;
    for (tick = 0; tick < ms; tick++) {
        for (i = 0; i < countPerMs; i++) {
            if (i > 10000000) {
                printf("!!\r\n");
            }
        }
    }
}

int32_t UpdateBootFromFlash(void)
{
    osKernelLock();
    int num = BinarySearchBootHead();
    printf("num = %d\n", num);
    if (num <= 0) {
        osKernelUnlock();
        return false;
    }
    uint32_t len, offset, crcCalc, readCrc, writeAddr = 0x1001000;
    uint32_t baseAddr = APP_ADDR + num * SECTOR_SIZE;
    int startNum = num + 1;
    uint8_t hash[32] = {0};
    uint8_t calHash[32] = {0};

    struct sha256_ctx ctx;
    sha256_init(&ctx);
    QspiFlashInit();

    memset(g_fileUnit, 0xFF, sizeof(g_fileUnit));

    memcpy(g_fileUnit, (uint32_t *)baseAddr, 4 + 32);
    uint32_t bootLen = (g_fileUnit[0] << 24) + (g_fileUnit[1] << 16) + (g_fileUnit[2] << 8) + g_fileUnit[3];
    printf("bootLen = %d\n", bootLen);
    memcpy(hash, &g_fileUnit[4], 32);

    memset(g_fileUnit, 0xFF, sizeof(g_fileUnit));
    memcpy(g_fileUnit, (uint32_t *)(baseAddr + 4 + 32 + 0x30), BOOT_HEAD_SIZE);
    QspiFlashEraseAndWrite(0x01000000, g_fileUnit, 4096);

    sha256_update(&ctx, (uint32_t *)(baseAddr + 4 + 32), 0x134);
    crcCalc = crc32_ieee(0, (uint32_t *)(baseAddr + 4 + 32), 0x134);

    for (int i = startNum; i <= startNum + (bootLen - 0x134) / SECTOR_SIZE; i++, writeAddr += SECTOR_SIZE) {
        Delay(100);
        memset(g_fileUnit, 0xFF, sizeof(g_fileUnit));
        if (i == startNum + (bootLen - 0x134) / SECTOR_SIZE) {
            len = (bootLen - 0x134) % SECTOR_SIZE - 4;
            sha256_update(&ctx, (uint32_t *)(APP_ADDR + i * SECTOR_SIZE), len + 4);
            memcpy(&readCrc, (uint32_t *)(APP_ADDR + i * SECTOR_SIZE + len), 4);
        } else {
            len = SECTOR_SIZE;
            sha256_update(&ctx, (uint32_t *)(APP_ADDR + i * SECTOR_SIZE), len);
        }
        memcpy(g_fileUnit, (uint32_t *)(APP_ADDR + i * SECTOR_SIZE), len);
        crcCalc = crc32_ieee(crcCalc, (uint32_t *)(APP_ADDR + i * SECTOR_SIZE), len);
        printf("writeAddr = %#x\n", writeAddr);
        QspiFlashEraseAndWrite(writeAddr, g_fileUnit, SECTOR_SIZE);
    }

    sha256_done(&ctx, (struct sha256 *)calHash);

    osKernelUnlock();
    if (memcmp(hash, calHash, 32) == 0) {
        printf("update success\n");
        memset(g_fileUnit, 0xFF, sizeof(g_fileUnit));
        QspiFlashEraseAndWrite((uint32_t *)(APP_END_ADDR - 4096), g_fileUnit, 4096);
        memset(g_fileUnit, 0, sizeof(g_fileUnit));
        return 0;
    } else {
        printf("update failed\n");
        return -1;
    }
}
