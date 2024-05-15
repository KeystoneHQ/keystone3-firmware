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
#include "user_memory.h"
#include "sha256.h"
#include "user_utils.h"
#include "librust_c.h"
#include "version.h"
#include "gui_views.h"
#include "gui_api.h"
#include "presetting.h"

#define CHECK_UNIT              256
#define CHECK_SIZE              4096

#define APP_VERSION_ADDR        0x01082000
#define APP_VERSION_HEAD        "Firmware v"
#define FILE_MARK_MCU_FIRMWARE              "~update!"
#define FILE_UNIT_SIZE                      0x4000
#define DATA_UNIT_SIZE                      0x4000
#define FIXED_SEGMENT_OFFSET                0x1000
#define UPDATE_PUB_KEY_LEN                  64
#define MAX_OTA_HEAD_SIZE                   0x100000

static char g_otaBinVersion[SOFTWARE_VERSION_MAX_LEN] = {0};

static uint32_t GetOtaFileInfo(OtaFileInfo_t *info, const char *filePath);
static int32_t CheckOtaFile(OtaFileInfo_t *info, const char *filePath, uint32_t *pHeadSize);
static int32_t CheckVersion(const OtaFileInfo_t *info, const char *filePath, uint32_t headSize, char *version);
int32_t GetSoftwareVersionFormData(uint32_t *major, uint32_t *minor, uint32_t *build, const uint8_t *data, uint32_t dataLen);
#if (SIGNATURE_ENABLE == 1)
static void GetSignatureValue(const cJSON *obj, char *output, uint32_t maxLength);
#endif

bool CheckApp(void)
{
    uint8_t read[4096];
    uint32_t major, minor, build;
    memcpy_s(read, 4096, (void *)APP_VERSION_ADDR, 4096);
    return GetSoftwareVersionFormData(&major, &minor, &build, read, 4096) == 0;
}

void GetSoftwareVersion(uint32_t *major, uint32_t *minor, uint32_t *build)
{
    uint8_t read[4096];

    memcpy_s(read, 4096, (void *)APP_VERSION_ADDR, 4096);
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
        memcpy_s(read, 64, &data[versionInfoOffset], 64);
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
    uint32_t headSize = 0, readSize, fileSize;
    char *headData = NULL;
    cJSON *jsonRoot;

    ret = f_open(&fp, filePath, FA_OPEN_EXISTING | FA_READ);
    do {
        if (ret) {
            FatfsError((FRESULT)ret);
            break;
        }
        fileSize = f_size(&fp);
        ret = f_read(&fp, &headSize, 4, (UINT *)&readSize);
        if (ret) {
            FatfsError((FRESULT)ret);
            break;
        }
        if (headSize > MAX_OTA_HEAD_SIZE) {
            printf("headSize>MAX,%d,%d\n", headSize, MAX_OTA_HEAD_SIZE);
            headSize = 0;
            break;
        }
        if (headSize > fileSize - 4) {
            printf("headSize>fileSize-4,%d,%d\n", headSize, fileSize);
            headSize = 0;
            break;
        }
        headData = SRAM_MALLOC(headSize + 2);
        ret = f_read(&fp, headData, headSize + 1, (UINT *)&readSize);
        if (ret) {
            headSize = 0;
            FatfsError((FRESULT)ret);
            break;
        }
        headData[headSize + 1] = 0;
        if (strlen(headData) != headSize) {
            printf("head err,str len=%d,headSize=%d\n", strlen(headData), headSize);
            headSize = 0;
            break;
        }
        jsonRoot = cJSON_Parse(headData);
        if (jsonRoot == NULL) {
            printf("parse error:%s\n", cJSON_GetErrorPtr());
            break;
        }
        cJSON *json = cJSON_GetObjectItem(jsonRoot, "mark");
        if (json != NULL) {
            if (strlen(json->valuestring) < OTA_FILE_INFO_MARK_MAX_LEN) {
                strcpy(info->mark, json->valuestring);
            } else {
                strcpy(info->mark, "");
            }
        } else {
            printf("key:%s does not exist\r\n", "mark");
        }
        printf("mark = %s\n", info->mark);

#if (SIGNATURE_ENABLE == 1)
        GetSignatureValue(jsonRoot, info->signature, SIGNATURE_LEN);
#endif
        info->fileSize = GetIntValue(jsonRoot, "fileSize", 0);
        info->originalFileSize = GetIntValue(jsonRoot, "originalFileSize", 0);
        info->crc32 = GetIntValue(jsonRoot, "crc32", 0);
        info->originalCrc32 = GetIntValue(jsonRoot, "originalCrc32", 0);
        info->encode = GetIntValue(jsonRoot, "encode", 0);
        info->encodeUnit = GetIntValue(jsonRoot, "encodeUnit", 0);
        info->encrypt = GetIntValue(jsonRoot, "encrypt", 0);
        info->originalBriefSize = GetIntValue(jsonRoot, "originalBriefSize", 0);
        info->originalBriefCrc32 = GetIntValue(jsonRoot, "originalBriefCrc32", 0);
        cJSON_Delete(jsonRoot);
    } while (0);
    f_close(&fp);
    if (headData != NULL) {
        SRAM_FREE(headData);
    }
    return headSize + 5;    //4 byte uint32 and 1 byte json string '\0' end.
}

static int32_t CheckOtaFile(OtaFileInfo_t *info, const char *filePath, uint32_t *pHeadSize)
{
    FIL fp;
    int32_t ret;
    uint32_t fileSize, crcCalc, readSize, i, headSize;
    struct sha256_ctx ctx;

    headSize = GetOtaFileInfo(info, filePath);
    *pHeadSize = headSize;

    ret = f_open(&fp, filePath, FA_OPEN_EXISTING | FA_READ);
    if (ret) {
        FatfsError((FRESULT)ret);
        return ERR_UPDATE_CHECK_FILE_EXIST;
    }
    fileSize = f_size(&fp);
    printf("mark=%s\r\n", info->mark);
    printf("fileSize=%d\r\n", info->fileSize);
    printf("originalFileSize=%d\r\n", info->originalFileSize);
    printf("crc32=0x%08X\r\n", info->crc32);
    printf("originalCrc32=0x%08X\r\n", info->originalCrc32);
    printf("encode=%d\r\n", info->encode);
    printf("encodeUnit=%d\r\n", info->encodeUnit);
    printf("encrypt=%d\r\n", info->encrypt);
    printf("destPath=%s\r\n", info->destPath);
    printf("originalBriefSize=%d\r\n", info->originalBriefSize);
    printf("originalBriefCrc32=0x%08X\r\n", info->originalBriefCrc32);
#if (SIGNATURE_ENABLE == 1)
    printf("signature=%s\r\n", info->signature);
#endif
    uint8_t *fileUnit = SRAM_MALLOC(FILE_UNIT_SIZE + 16);
    ret = SUCCESS_CODE;
    uint8_t percent = 0;
    do {
        if (fileSize != info->fileSize + headSize) {
            printf("file size err,fileSize=%d, info->fileSize=%d\r\n", fileSize, info->fileSize);
            ret = ERR_UPDATE_CHECK_CRC_FAILED;
            break;
        }
        if (strcmp(info->mark, FILE_MARK_MCU_FIRMWARE) != 0) {
            printf("file info mark err\r\n");
            ret = ERR_UPDATE_CHECK_CRC_FAILED;
            break;
        }
        printf("start to check file crc32\r\n");
        f_lseek(&fp, headSize);
        crcCalc = 0;

        sha256_init(&ctx);
        uint8_t contentHash[32];
        for (i = headSize; i < fileSize; i += readSize) {
            ret = f_read(&fp, fileUnit, FILE_UNIT_SIZE, (UINT *)&readSize);
            if (ret) {
                FatfsError((FRESULT)ret);
                percent = 0xFD; 
                GuiApiEmitSignal(SIG_SETTING_VERIFY_OTA_PERCENT, &percent, sizeof(percent));
                f_close(&fp);
                return ERR_UPDATE_CHECK_CRC_FAILED;
            }
            //printf("i=%d,readSize=%d\r\n", i, readSize);
            crcCalc = crc32_ieee(crcCalc, fileUnit, readSize);
            sha256_update(&ctx, fileUnit, readSize);
            if (percent != i * 100 / fileSize) {
                percent = i * 100 / fileSize;
                if (percent <= 98) {
                    GuiApiEmitSignal(SIG_SETTING_VERIFY_OTA_PERCENT, &percent, sizeof(percent));
                }
            }
        }
        percent = 99;
        GuiApiEmitSignal(SIG_SETTING_VERIFY_OTA_PERCENT, &percent, sizeof(percent));
        sha256_done(&ctx, (struct sha256 *)contentHash);
        PrintArray("hash content:", contentHash, 32);
        if (crcCalc != info->crc32) {
            printf("crc err,crcCalc=0x%08X,info->crc32=0x%08X\r\n", crcCalc, info->crc32);
            percent = 0xFE; 
            GuiApiEmitSignal(SIG_SETTING_VERIFY_OTA_PERCENT, &percent, sizeof(percent));
            ret = ERR_UPDATE_CHECK_CRC_FAILED;
            break;
        }

#if (SIGNATURE_ENABLE == 1)
        printf("signature=%s\r\n", info->signature);
        if (strlen(info->signature) != 128) {
            printf("error signature=%s\r\n", info->signature);
            percent = 0xFF;
            GuiApiEmitSignal(SIG_SETTING_VERIFY_OTA_PERCENT, &percent, sizeof(percent));
            ret = ERR_UPDATE_CHECK_SIGNATURE_FAILED;
            break;
        }
        // TODO: find this public key from firmware section.
        uint8_t publickey[65] = {0};
        GetUpdatePubKey(publickey);
        PrintArray("pubKey", publickey, 65);
        if (verify_frimware_signature(info->signature, contentHash, publickey) != true) {
            printf("signature check error\n");
            percent = 0xFF;
            GuiApiEmitSignal(SIG_SETTING_VERIFY_OTA_PERCENT, &percent, sizeof(percent));
            ret = ERR_UPDATE_CHECK_SIGNATURE_FAILED;
            break;
        }
#endif
    } while (0);
    f_close(&fp);

    return ret;
}

bool GetOtaBinVersion(char *version, uint32_t maxLen)
{
    int len = strnlen_s(g_otaBinVersion, SOFTWARE_VERSION_MAX_LEN);
    if (len == 0) {
        return false;
    }
    strncpy_s(version, maxLen, g_otaBinVersion, len);
    return true;
}

bool CheckOtaBinVersion(void)
{
    OtaFileInfo_t otaFileInfo = {0};
    uint32_t headSize;
    int32_t ret = SUCCESS_CODE;
    memset_s(g_otaBinVersion, SOFTWARE_VERSION_MAX_LEN, 0, SOFTWARE_VERSION_MAX_LEN);

    do {
        ret = CheckOtaFile(&otaFileInfo, SD_CARD_OTA_BIN_PATH, &headSize);
        if (ret != SUCCESS_CODE) {
            break;
        }

        ret = CheckVersion(&otaFileInfo, SD_CARD_OTA_BIN_PATH, headSize, g_otaBinVersion);
        uint8_t percent = 100;
        GuiApiEmitSignal(SIG_SETTING_VERIFY_OTA_PERCENT, &percent, sizeof(percent));
        if (ret != SUCCESS_CODE) {
            printf("file %s version err\n", SD_CARD_OTA_BIN_PATH);
            break;
        }
    } while (0);
    if (ret == SUCCESS_CODE) {
        return true;
    }
    return false;
}

static int32_t CheckVersion(const OtaFileInfo_t *info, const char *filePath, uint32_t headSize, char *version)
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
        return ERR_UPDATE_CHECK_VERSION_FAILED;
    }
    f_lseek(&fp, headSize);
    ret = f_read(&fp, g_fileUnit, 16, (UINT *)&readSize);
    if (ret) {
        FatfsError((FRESULT)ret);
        f_close(&fp);
        return ERR_UPDATE_CHECK_VERSION_FAILED;
    }
    cmpsdSize = qlz_size_compressed((char*)g_fileUnit);
    decmpsdSize = qlz_size_decompressed((char*)g_fileUnit);
    printf("cmpsdSize=%d,decmpsdSize=%d\r\n", cmpsdSize, decmpsdSize);
    ret = f_read(&fp, g_fileUnit + 16, cmpsdSize - 16, (UINT *)&readSize);
    if (ret) {
        FatfsError((FRESULT)ret);
        f_close(&fp);
        return ERR_UPDATE_CHECK_VERSION_FAILED;
    }
    f_close(&fp);
    qlz_decompress((char*)g_fileUnit, g_dataUnit, &qlzState);
    GetSoftwareVersion(&nowMajor, &nowMinor, &nowBuild);
    ret = GetSoftwareVersionFormData(&fileMajor, &fileMinor, &fileBuild, g_dataUnit + FIXED_SEGMENT_OFFSET, decmpsdSize - FIXED_SEGMENT_OFFSET);
    if (ret != 0) {
        SRAM_FREE(g_dataUnit);
        SRAM_FREE(g_fileUnit);
        return ERR_UPDATE_CHECK_VERSION_FAILED;
    }
    printf("now version:%d.%d.%d\n", nowMajor, nowMinor, nowBuild);
    printf("file version:%d.%d.%d\n", fileMajor, fileMinor, fileBuild);

    uint32_t epoch = 100;
    uint32_t nowVersionNumber = (nowMajor * epoch * epoch)  + (nowMinor * epoch) + nowBuild;
    uint32_t fileVersionNumber = (fileMajor * epoch * epoch)  + (fileMinor * epoch) + fileBuild;

    SRAM_FREE(g_dataUnit);
    SRAM_FREE(g_fileUnit);
    if (fileVersionNumber <= nowVersionNumber) {
        return ERR_UPDATE_CHECK_VERSION_FAILED;
    }
    if (fileMajor >= 10) {
        fileMajor = fileMajor % 10;
        snprintf_s(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d-BTC", fileMajor, fileMinor, fileBuild);
    } else {
        snprintf_s(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d", fileMajor, fileMinor, fileBuild);
    }
    return SUCCESS_CODE;
}

static void GetSignatureValue(const cJSON *obj, char *output, uint32_t maxLength)
{
    cJSON *signatureValue = cJSON_GetObjectItem((cJSON *)obj, "signature");
    if (signatureValue->type == cJSON_String) {
        char *strTemp = signatureValue->valuestring;
        strncpy(output, strTemp, maxLength);
        return;
    }
    memset(output, 0, maxLength);
    printf("signature does not exist\r\n");
}
