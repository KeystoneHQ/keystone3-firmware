#include "firmware_update.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "user_fatfs.h"
#include "ff.h"
#include "crc.h"
#include "quicklz.h"
#include "log_print.h"
#include "cjson/cJSON.h"
#include "hal_lcd.h"
#include "user_delay.h"
#include "draw_on_lcd.h"
#include "drv_power.h"
#include "drv_lcd_bright.h"
#include "user_memory.h"
#include "sha256.h"
#include "mhscpu.h"
#include "drv_otp.h"
#include "user_utils.h"

#define SD_CARD_OTA_BIN_PATH   "0:/keystone3.bin"

#define CHECK_UNIT              256
#define CHECK_SIZE              4096

#define APP_VERSION_ADDR        0x01082000
#define APP_VERSION_HEAD        "Firmware v"

LV_FONT_DECLARE(openSans_20);
LV_FONT_DECLARE(openSans_24);


#define FILE_MARK_MCU_FIRMWARE              "~update!"

#define FILE_UNIT_SIZE                      0x4000
#define DATA_UNIT_SIZE                      0x4000

#define FIXED_SEGMENT_OFFSET                0x1000

#define UPDATE_PUB_KEY_LEN                  64

static uint32_t GetOtaFileInfo(OtaFileInfo_t *info, const char *filePath);
static bool CheckOtaFile(OtaFileInfo_t *info, const char *filePath, uint32_t *pHeadSize);
static bool CheckVersion(const OtaFileInfo_t *info, const char *filePath, uint32_t headSize, char *version);

int32_t GetSoftwareVersionFormData(uint32_t *major, uint32_t *minor, uint32_t *build, const uint8_t *data, uint32_t dataLen);

bool CheckApp(void)
{
    uint8_t read[4096];
    uint32_t major, minor, build;
    memcpy(read, (void *)APP_VERSION_ADDR, 4096);
    return GetSoftwareVersionFormData(&major, &minor, &build, read, 4096) == 0;
}

void GetSoftwareVersion(uint32_t *major, uint32_t *minor, uint32_t *build)
{
    uint8_t read[4096];

    memcpy(read, (void *)APP_VERSION_ADDR, 4096);
    GetSoftwareVersionFormData(major, minor, build, read, 4096);
}

int32_t GetSoftwareVersionFormData(uint32_t *major, uint32_t *minor, uint32_t *build, const uint8_t *data, uint32_t dataLen)
{
    uint32_t versionInfoOffset = UINT32_MAX, i, headLen;
    char *versionStr, read[64];
    int32_t ret;
    bool succ = false;

    headLen = strlen(APP_VERSION_HEAD);
    for (i = 0; i < dataLen - headLen - 32; i++) {
        if (data[i] == 'F') {
            if (strncmp((char *)&data[i], APP_VERSION_HEAD, headLen) == 0) {
                versionInfoOffset = i;
                break;
            }
        }
    }
    do {
        if (versionInfoOffset == UINT32_MAX) {
            printf("version string not found in fixed segment\n");
            break;
        }
        memcpy(read, &data[versionInfoOffset], 64);
        read[31] = '\0';
        if (strncmp(read, APP_VERSION_HEAD, headLen) != 0) {
            break;
        }
        versionStr = read + headLen;
        //printf("versionStr=%s\n", versionStr);
        ret = sscanf(versionStr, "%d.%d.%d", major, minor, build);
        if (ret != 3) {
            break;
        }
        succ = true;
    } while (0);
    if (succ == false) {
        *major = 0;
        *minor = 0;
        *build = 0;
    }
    return succ ? 0 : -1;
}


/// @brief
/// @param info
/// @param filePath
/// @return head size, as same as data index.
static uint32_t GetOtaFileInfo(OtaFileInfo_t *info, const char *filePath)
{
    FIL fp;
    int32_t ret;
    uint32_t headSize = 0, readSize;

    ret = f_open(&fp, filePath, FA_OPEN_EXISTING | FA_READ);
    do {
        if (ret) {
            FatfsError((FRESULT)ret);
            break;
        }
        ret = f_read(&fp, &headSize, 4, (UINT *)&readSize);
        if (ret) {
            FatfsError((FRESULT)ret);
            break;
        }
    } while (0);
    f_close(&fp);
    return headSize + 5;    //4 byte uint32 and 1 byte json string '\0' end.
}

static bool CheckOtaFile(OtaFileInfo_t *info, const char *filePath, uint32_t *pHeadSize)
{
    uint32_t headSize;

    headSize = GetOtaFileInfo(info, filePath);
    *pHeadSize = headSize;

    return !(headSize <= 5);
}

bool CheckOtaBinVersion(char *version)
{
    // strcpy(version, "99.99.99");
    // return true;
    OtaFileInfo_t otaFileInfo = {0};
    uint32_t headSize;
    bool ret = true;

    do {
        ret = CheckOtaFile(&otaFileInfo, SD_CARD_OTA_BIN_PATH, &headSize);
        if (ret == false) {
            break;
        }

        ret = CheckVersion(&otaFileInfo, SD_CARD_OTA_BIN_PATH, headSize, version);
        if (ret == false) {
            printf("file %s version err\n", SD_CARD_OTA_BIN_PATH);
            break;
        }
    } while (0);
    return ret;
}

static bool CheckVersion(const OtaFileInfo_t *info, const char *filePath, uint32_t headSize, char *version)
{
    uint8_t *g_fileUnit = SRAM_MALLOC(FILE_UNIT_SIZE + 16);
    uint8_t *g_dataUnit = SRAM_MALLOC(DATA_UNIT_SIZE);

    FIL fp;
    int32_t ret;
    uint32_t readSize, cmpsdSize, decmpsdSize;
    qlz_state_decompress qlzState = {0};
    uint32_t nowMajor, nowMinor, nowBuild;
    uint32_t fileMajor, fileMinor, fileBuild;

    ret = f_open(&fp, filePath, FA_OPEN_EXISTING | FA_READ);
    if (ret) {
        FatfsError((FRESULT)ret);
        return false;
    }
    f_lseek(&fp, headSize);
    ret = f_read(&fp, g_fileUnit, 16, (UINT *)&readSize);
    if (ret) {
        FatfsError((FRESULT)ret);
        f_close(&fp);
        return false;
    }
    cmpsdSize = qlz_size_compressed((char*)g_fileUnit);
    decmpsdSize = qlz_size_decompressed((char*)g_fileUnit);
    printf("cmpsdSize=%d,decmpsdSize=%d\r\n", cmpsdSize, decmpsdSize);
    ret = f_read(&fp, g_fileUnit + 16, cmpsdSize - 16, (UINT *)&readSize);
    if (ret) {
        FatfsError((FRESULT)ret);
        f_close(&fp);
        return false;
    }
    f_close(&fp);
    qlz_decompress((char*)g_fileUnit, g_dataUnit, &qlzState);
    GetSoftwareVersion(&nowMajor, &nowMinor, &nowBuild);
    ret = GetSoftwareVersionFormData(&fileMajor, &fileMinor, &fileBuild, g_dataUnit + FIXED_SEGMENT_OFFSET, decmpsdSize - FIXED_SEGMENT_OFFSET);
    if (ret != 0) {
        SRAM_FREE(g_dataUnit);
        SRAM_FREE(g_fileUnit);
        return false;
    }
    printf("now version:%d.%d.%d\n", nowMajor, nowMinor, nowBuild);
    printf("file version:%d.%d.%d\n", fileMajor, fileMinor, fileBuild);

    uint32_t epoch = 100;
    uint32_t nowVersionNumber = (nowMajor * epoch * epoch)  + (nowMinor * epoch) + nowBuild;
    uint32_t fileVersionNumber = (fileMajor * epoch * epoch)  + (fileMinor * epoch) + fileBuild;

    if (fileMajor >= 10) {
        fileMajor = fileMajor % 10;
        sprintf(version, "%d.%d.%d-BTC", fileMajor, fileMinor, fileBuild);
    } else {
        sprintf(version, "%d.%d.%d", fileMajor, fileMinor, fileBuild);
    }
    if (fileVersionNumber <= nowVersionNumber) {
        SRAM_FREE(g_dataUnit);
        SRAM_FREE(g_fileUnit);
        return false;
    }
    SRAM_FREE(g_dataUnit);
    SRAM_FREE(g_fileUnit);
    return true;
}


