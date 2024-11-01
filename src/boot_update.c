#include "define.h"
#include "mhscpu.h"
#include "user_fatfs.h"
#include "user_memory.h"
#include "draw_on_lcd.h"
#include "drv_qspi_flash.h"

LV_FONT_DECLARE(openSans_24);

#define FILE_UNIT_SIZE                      0x4000
#define BOOT_ADDR                           (0x01001000)
#define APP_ADDR                            (0x1001000 + 0x80000 + 0x80000)   //108 1000
#define SECTOR_SIZE                         4096
#define BOOT_HEAD_SIZE                      0x104

static void UpdateBootFromOtaFile(const char *filePath)
{
    FIL fp;
    int32_t ret;
    uint32_t fileSize, readSize, len, offset;
    static uint32_t g_fileSize = 0, g_lastSize = 0;
    static uint32_t lastPercent = 101;

    ret = f_open(&fp, filePath, FA_OPEN_EXISTING | FA_READ);
    if (ret) {
        FatfsError((FRESULT)ret);
        return;
    }
    uint8_t *readBuf = SRAM_MALLOC(4096);
    DrawStringOnLcd(120, 412, "Bootloader Updating...", 0xFFFF, &openSans_24);
    QspiFlashInit();
    fileSize = f_size(&fp);
    g_lastSize = fileSize - 0x134 - 4;
    g_fileSize = fileSize - 0x134 - 4;
    uint32_t crcCalc = 0;
    uint32_t writeAddr = 0x1001000;
    ret = f_read(&fp, readBuf, 0x134, (UINT *)&readSize);
    crcCalc = crc32_ieee(0, readBuf, readSize);

    memset(readBuf, 0xFF, sizeof(readBuf));
    memcpy(readBuf, &readBuf[0x30], BOOT_HEAD_SIZE);
    QspiFlashEraseAndWrite(0x01000000, readBuf, 4096);

    while (g_lastSize > 0) {
        UserDelay(100);
        memset_s(readBuf, sizeof(readBuf), 0xFF, sizeof(readBuf));
        len = g_lastSize > 4096 ? 4096 : g_lastSize;
        ret = f_read(&fp, readBuf, len, (UINT *)&readSize);
        g_lastSize -= len;
        QspiFlashEraseAndWrite(writeAddr, readBuf, 4096);
        writeAddr += 4096;
        crcCalc = crc32_ieee(crcCalc, readBuf, readSize);
        // sprintf(percentStr, "%d%%", (g_fileSize - g_lastSize) * 100 / g_fileSize);
        // DrawStringOnLcd(210, 466, (char *)percentStr, 0xFFFF, &openSans_24);
    }
    uint32_t readCrc = 0;
    ret = f_read(&fp, &readCrc, 4, (UINT *)&readSize);
    f_close(&fp);
    printf("write over\n");
    f_unlink(filePath);
    SRAM_FREE(readBuf);
    NVIC_SystemReset();
}

void BootUpdate(char *filePath)
{
    UpdateBootFromOtaFile(filePath);
}
