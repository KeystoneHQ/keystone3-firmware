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
#include "version.h"

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
}

void GetSoftwareVersion(uint32_t *major, uint32_t *minor, uint32_t *build)
{
}

int32_t GetSoftwareVersionFormData(uint32_t *major, uint32_t *minor, uint32_t *build, const uint8_t *data, uint32_t dataLen)
{
}


/// @brief
/// @param info
/// @param filePath
/// @return head size, as same as data index.
static uint32_t GetOtaFileInfo(OtaFileInfo_t *info, const char *filePath)
{
}

static bool CheckOtaFile(OtaFileInfo_t *info, const char *filePath, uint32_t *pHeadSize)
{

}

bool CheckOtaBinVersion(char *version)
{

}

static bool CheckVersion(const OtaFileInfo_t *info, const char *filePath, uint32_t headSize, char *version)
{
    return true;
}


