#include "define.h"
#include "mhscpu.h"
#include "user_fatfs.h"
#include "user_memory.h"


#define FILE_UNIT_SIZE                      0x4000
#define BOOT_ADDR                           (0x01001000)
#define APP_ADDR                            (0x1001000 + 0x80000 + 0x80000)   //108 1000
#define SECTOR_SIZE                         4096
#define BOOT_HEAD_SIZE                      0x104
static uint8_t g_readBuf[4096] = {0};

static void UpdateBootFromOtaFile(const char *filePath)
{
    FIL fp;
    int32_t ret;
    uint32_t fileSize, readSize, len, offset;
    static uint32_t g_fileSize = 0, g_lastSize = 0;
    static uint32_t lastPercent = 101;
    char percentStr[16];
    char bootHeadBuf[0x134] = {0};

    ret = f_open(&fp, filePath, FA_OPEN_EXISTING | FA_READ);
    if (ret) {
        FatfsError((FRESULT)ret);
        return;
    }
    QspiFlashInit();
    fileSize = f_size(&fp);
    g_lastSize = fileSize - 0x134 - 4;
    g_fileSize = fileSize - 0x134 - 4;
    uint32_t crcCalc = 0;
    uint32_t writeAddr = 0x1001000;
    ret = f_read(&fp, bootHeadBuf, 0x134, (UINT *)&readSize);
    crcCalc = crc32_ieee(0, bootHeadBuf, readSize);

    memset(g_readBuf, 0xFF, sizeof(g_readBuf));
    memcpy(g_readBuf, &bootHeadBuf[0x30], BOOT_HEAD_SIZE);
    QspiFlashEraseAndWrite(0x01000000, g_readBuf, 4096);

    while (g_lastSize > 0) {
        UserDelay(100);
        memset_s(g_readBuf, sizeof(g_readBuf), 0xFF, sizeof(g_readBuf));
        len = g_lastSize > 4096 ? 4096 : g_lastSize;
        ret = f_read(&fp, g_readBuf, len, (UINT *)&readSize);
        g_lastSize -= len;
        QspiFlashEraseAndWrite(writeAddr, g_readBuf, 4096);
        writeAddr += 4096;
        crcCalc = crc32_ieee(crcCalc, g_readBuf, readSize);
    }
    uint32_t readCrc = 0;
    ret = f_read(&fp, &readCrc, 4, (UINT *)&readSize);
    printf("write over\n");
}

void BootUpdate(char *filePath)
{
    int32_t ret = SUCCESS_CODE;
    const char *file = "1:mh1903_boot.sig";
    UpdateBootFromOtaFile(file);
    f_unlink(filePath);
    NVIC_SystemReset();
}
