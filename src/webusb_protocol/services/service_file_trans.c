#include "service_file_trans.h"
#include "stdio.h"
#include "protocol_codec.h"
#include "string.h"
#include "log_print.h"
#include "user_utils.h"
#include "assert.h"
#include "md5.h"
#include "cmsis_os.h"
#include "user_fatfs.h"
#include "ff.h"
#include "gui_views.h"
#include "gui_api.h"
#include "librust_c.h"
#include "sha256.h"
#include "background_task.h"
#include "drv_battery.h"
#include "keystore.h"
#include "device_setting.h"
#include "account_manager.h"
#include "user_memory.h"
#include "drv_gd25qxx.h"

#define TYPE_FILE_INFO_FILE_NAME                        1
#define TYPE_FILE_INFO_FILE_SIZE                        2
#define TYPE_FILE_INFO_FILE_MD5                         3
#define TYPE_FILE_INFO_FILE_SIGN                        4

#define TYPE_FILE_INFO_FILE_OFFSET                      1
#define TYPE_FILE_INFO_FILE_DATA                        2
#define TYPE_FILE_INFO_FILE_ACK                         3

#define MAX_FILE_NAME_LENGTH                            32

#define FILE_TRANS_TIME_OUT                             2000

typedef struct {
    char fileName[MAX_FILE_NAME_LENGTH + 4];
    uint32_t fileSize;
    uint8_t md5[16];
    uint8_t signature[64];
} FileTransInfo_t;

typedef struct  {
    uint32_t startTick;
    uint32_t endTick;
    uint32_t offset;
} FileTransCtrl_t;

static MD5_CTX ctx;

static uint8_t *ServiceFileTransInfo(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static uint8_t *ServiceFileTransContent(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static uint8_t *GetFileContent(const FrameHead_t *head, uint32_t offset, uint32_t *outLen);
static uint8_t *ServiceFileTransComplete(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
#ifdef BTC_ONLY
static uint8_t *ServiceNftFileTransInfo(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static uint8_t *ServiceNftFileTransContent(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static uint8_t *ServiceNftFileTransComplete(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static bool g_isNftFile = false;

const ProtocolServiceCallbackFunc_t g_nftFileTransInfoServiceFunc[] = {
    NULL,                                       //3.0
    ServiceNftFileTransInfo,                    //3.1
    ServiceNftFileTransContent,                 //3.2
    ServiceNftFileTransComplete,                //3.3
};
#endif

static bool g_isReceivingFile = false;

bool GetIsReceivingFile()
{
    return g_isReceivingFile;
}

static void FileTransTimeOutTimerFunc(void *argument);

static FileTransInfo_t g_fileTransInfo;
static FileTransCtrl_t g_fileTransCtrl;
static osTimerId_t g_fileTransTimeOutTimer = NULL;

static const uint8_t g_webUsbPubKey[] = {
    0x04, 0x85, 0x1C, 0xD8, 0x8D, 0xCE, 0xB1, 0xAF, 0xBB, 0xCC, 0x8E, 0x0A, 0xF3, 0xC1, 0x7E, 0x61, \
    0x15, 0xD4, 0x38, 0x4E, 0xB8, 0xD1, 0xB3, 0x02, 0xFC, 0xE3, 0xD0, 0xAB, 0xAC, 0x9C, 0x15, 0x43, \
    0x75, 0xAD, 0xBE, 0x60, 0x95, 0xFC, 0xC9, 0x4C, 0x75, 0x75, 0x88, 0x02, 0xEC, 0x0E, 0x25, 0xF3, \
    0xE0, 0x8D, 0xCC, 0x38, 0x9D, 0xCB, 0x25, 0xA4, 0xCA, 0x2A, 0x52, 0x3D, 0x3E, 0x7B, 0xB3, 0xDD, \
    0x5E
};

const ProtocolServiceCallbackFunc_t g_fileTransInfoServiceFunc[] = {
    NULL,                                       //2.0
    ServiceFileTransInfo,                       //2.1
    ServiceFileTransContent,                    //2.2
    ServiceFileTransComplete,                   //2.3
};

static int ValidateAndSetFileName(Tlv_t *tlvArray, FileTransInfo_t *fileTransInfo)
{
    if (tlvArray == NULL || fileTransInfo == NULL) {
        printf("Invalid pointers provided.\n");
        return -1;
    }

    size_t pValueLength = strnlen_s(tlvArray->pValue, MAX_FILE_NAME_LENGTH);
    ASSERT((pValueLength + 1) == tlvArray->length);
    if (pValueLength >= MAX_FILE_NAME_LENGTH || tlvArray->length > MAX_FILE_NAME_LENGTH) {
        printf("File name is too long.\n");
        return -1;
    }

    if (strcmp("keystone3.bin", tlvArray->pValue) != 0) {
        return -1;
    }
    int written = snprintf(fileTransInfo->fileName, MAX_FILE_NAME_LENGTH + 3, "1:%s", tlvArray->pValue);
    if (written < 0 || written >= MAX_FILE_NAME_LENGTH + 3) {
        printf("Failed to write file name.\n");
        return -1;
    }

    return SUCCESS_CODE;
}

static uint8_t *ServiceFileTransInfo(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    Tlv_t tlvArray[5];
    Tlv_t sendTlvArray[1] = {0};
    uint32_t tlvNumber;
    FrameHead_t sendHead = {0};
    uint8_t hash[32];
    g_isReceivingFile = true;
    printf("ServiceFileTransInfo\n");
    tlvNumber = GetTlvFromData(tlvArray, 5, tlvData, head->length);
    CLEAR_OBJECT(g_fileTransInfo);
    CLEAR_OBJECT(g_fileTransCtrl);
    for (uint32_t i = 0; i < tlvNumber; i++) {
        switch (tlvArray[i].type) {
        case TYPE_FILE_INFO_FILE_NAME: {
            if (ValidateAndSetFileName(&tlvArray[i], &g_fileTransInfo) != 0) {
                return NULL;
            }
        }
        break;
        case TYPE_FILE_INFO_FILE_SIZE: {
            if (tlvArray[i].length != 4) {
                printf("file size err\n");
                return NULL;
            }
            g_fileTransInfo.fileSize = *(uint32_t *)tlvArray[i].pValue;
        }
        break;
        case TYPE_FILE_INFO_FILE_MD5: {
            if (tlvArray[i].length != 16) {
                printf("file md5 err\n");
                return NULL;
            }
            memcpy_s(g_fileTransInfo.md5, sizeof(g_fileTransInfo.md5), tlvArray[i].pValue, 16);
        }
        break;
        case TYPE_FILE_INFO_FILE_SIGN: {
            if (tlvArray[i].length != 64) {
                printf("file signature err\n");
                return NULL;
            }
            memcpy_s(g_fileTransInfo.signature, sizeof(g_fileTransInfo.signature), tlvArray[i].pValue, 64);
        }
        break;
        default:
            break;
        }
    }
    printf("file name=%s\n", g_fileTransInfo.fileName);
    printf("file size=%d\n", g_fileTransInfo.fileSize);
    PrintArray("md5", g_fileTransInfo.md5, 16);
    PrintArray("signature", g_fileTransInfo.signature, 64);

    do {
        if (strnlen_s(g_fileTransInfo.fileName, MAX_FILE_NAME_LENGTH) == 0 || g_fileTransInfo.fileSize == 0) {
            sendTlvArray[0].value = 4;
            break;
        }
        sha256((struct sha256 *)hash, g_fileTransInfo.md5, 16);
        PrintArray("hash", hash, 32);
        if (k1_verify_signature(g_fileTransInfo.signature, hash, (uint8_t *)g_webUsbPubKey) == false) {
            printf("verify signature fail\n");
            sendTlvArray[0].value = 3;
            break;
        }
        printf("verify signature ok\n");
        uint8_t walletAmount;
        GetExistAccountNum(&walletAmount);
        if (GetCurrentAccountIndex() == ACCOUNT_INDEX_LOGOUT && walletAmount != 0) {
            GuiApiEmitSignalWithValue(SIG_INIT_FIRMWARE_UPDATE_DENY, 1);
            sendTlvArray[0].value = 2;
            break;
        }
        if (GetCurrentDisplayPercent() < LOW_BATTERY_PERCENT) {
            GuiApiEmitSignalWithValue(SIG_INIT_LOW_BATTERY, 1);
            sendTlvArray[0].value = 1;
            break;
        }
        g_fileTransCtrl.startTick = osKernelGetTickCount();
        MD5_Init(&ctx);
        if (FatfsFileCreate(g_fileTransInfo.fileName) != RES_OK) {
            printf("create file %s err\n", g_fileTransInfo.fileName);
            sendTlvArray[0].value = 5;
            break;
        }
#ifdef BTC_ONLY
        GuiApiEmitSignalWithValue(g_isNftFile ? SIG_INIT_NFT_BIN : SIG_INIT_FIRMWARE_PROCESS, 1);
#else
        GuiApiEmitSignalWithValue(SIG_INIT_FIRMWARE_PROCESS, 1);
#endif
        if (g_fileTransTimeOutTimer == NULL) {
            g_fileTransTimeOutTimer = osTimerNew(FileTransTimeOutTimerFunc, osTimerOnce, NULL, NULL);
        }
        g_isReceivingFile = true;
        osTimerStart(g_fileTransTimeOutTimer, FILE_TRANS_TIME_OUT);
    } while (0);

    sendHead.packetIndex = head->packetIndex;
    sendHead.serviceId = head->serviceId;
    sendHead.commandId = head->commandId;
    sendHead.flag.b.ack = 0;
    sendHead.flag.b.isHost = 0;

    sendTlvArray[0].type = TYPE_GENERAL_RESULT_ACK;
    sendTlvArray[0].length = 4;

    *outLen = GetFrameTotalLength(sendTlvArray, 1);
    return BuildFrame(&sendHead, sendTlvArray, 1);
}

static uint8_t *ServiceFileTransContent(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    Tlv_t tlvArray[2] = {0};
    uint32_t tlvNumber, offset = UINT32_MAX, fileDataSize = UINT32_MAX;
    uint8_t *fileData = NULL;

    ASSERT(g_fileTransTimeOutTimer);
    osTimerStart(g_fileTransTimeOutTimer, FILE_TRANS_TIME_OUT);
    //PrintArray("tlvData", tlvData, head->length);
    tlvNumber = GetTlvFromData(tlvArray, 2, tlvData, head->length);
    //printf("tlvNumber=%d\n", tlvNumber);
    for (uint32_t i = 0; i < tlvNumber; i++) {
        switch (tlvArray[i].type) {
        case TYPE_FILE_INFO_FILE_OFFSET: {
            if (tlvArray[i].length != 4) {
                printf("offset err\n");
                return NULL;
            }
            offset = *(uint32_t *)tlvArray[i].pValue;
            printf("offset=%d\n", offset);
        }
        break;
        case TYPE_FILE_INFO_FILE_DATA: {
            //printf("file data len=%d\n", tlvArray[i].length);
            fileData = tlvArray[i].pValue;
            fileDataSize = tlvArray[i].length;
        }
        break;
        default:
            break;
        }
    }
    if (fileData == NULL || offset == UINT32_MAX || fileDataSize == UINT32_MAX) {
        return NULL;
    }
    if (g_fileTransCtrl.offset != offset) {
        printf("file trans offset err, expected offset=%d, rcv offset=%d\n", g_fileTransCtrl.offset, offset);
        return NULL;
    }
    g_fileTransCtrl.offset += fileDataSize;
    if (FatfsFileAppend(g_fileTransInfo.fileName, fileData, fileDataSize) != RES_OK) {
        printf("append file %s err\n", g_fileTransInfo.fileName);
        return NULL;
    }
    MD5_Update(&ctx, fileData, fileDataSize);
    return GetFileContent(head, g_fileTransCtrl.offset, outLen);
}

static uint8_t *GetFileContent(const FrameHead_t *head, uint32_t offset, uint32_t *outLen)
{
    Tlv_t tlvArray[1] = {0};
    FrameHead_t sendHead = {0};

    sendHead.packetIndex = head->packetIndex;
    // sendHead.serviceId = SERVICE_ID_FILE_TRANS;
#ifdef BTC_ONLY
    sendHead.serviceId = g_isNftFile ? SERVICE_ID_NFT_FILE_TRANS : SERVICE_ID_FILE_TRANS;
#else
    sendHead.serviceId = SERVICE_ID_FILE_TRANS;
#endif
    sendHead.commandId = COMMAND_ID_FILE_TRANS_CONTENT;
    sendHead.flag.b.ack = 0;
    sendHead.flag.b.isHost = 0;

    tlvArray[0].type = TYPE_FILE_INFO_FILE_ACK;
    tlvArray[0].length = 4;
    tlvArray[0].value = offset;

    *outLen = GetFrameTotalLength(tlvArray, 1);
    return BuildFrame(&sendHead, tlvArray, 1);
}

static uint8_t *ServiceFileTransComplete(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    FrameHead_t sendHead = {0};
    uint8_t md5Result[16];

    ASSERT(g_fileTransTimeOutTimer);
    osTimerStop(g_fileTransTimeOutTimer);
    g_fileTransCtrl.endTick = osKernelGetTickCount();
    PrintArray("tlvData", tlvData, head->length);
    MD5_Final(md5Result, &ctx);
    PrintArray("g_fileTransInfo.md5", g_fileTransInfo.md5, 16);
    PrintArray("md5Result", md5Result, 16);
    ASSERT(memcmp(md5Result, g_fileTransInfo.md5, 16) == 0);
    printf("total tick=%d\n", g_fileTransCtrl.endTick - g_fileTransCtrl.startTick);

    sendHead.packetIndex = head->packetIndex;
    sendHead.serviceId = head->serviceId;
    sendHead.commandId = head->commandId;
    sendHead.flag.b.ack = 0;
    sendHead.flag.b.isHost = 0;

    GuiApiEmitSignalWithValue(SIG_INIT_FIRMWARE_PROCESS, 0);
    *outLen = sizeof(FrameHead_t) + 4;
    SetSetupStep(4);
    SaveDeviceSettings();
    SystemReboot();
    return BuildFrame(&sendHead, NULL, 0);
}

static void FileTransTimeOutTimerFunc(void *argument)
{
    g_isReceivingFile = false;
    GuiApiEmitSignalWithValue(SIG_INIT_FIRMWARE_PROCESS, 0);
#ifdef BTC_ONLY
    if (g_isNftFile) {
        GuiApiEmitSignalWithValue(SIG_INIT_NFT_BIN, 0);
        GuiApiEmitSignalWithValue(SIG_INIT_NFT_BIN_TRANS_FAIL, 0);
    }
    g_isNftFile = false;
#endif
}

#ifdef BTC_ONLY
static uint8_t *ServiceNftFileTransInfo(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    g_isNftFile = true;
    SetNftBinValid(false);
    SaveDeviceSettings();
    return ServiceFileTransInfo(head, tlvData, outLen);
}

static uint8_t *ServiceNftFileTransContent(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    return ServiceFileTransContent(head, tlvData, outLen);
}

#define START_ADDR 0x00EB2000
static void WriteNftToFlash(void)
{
    FIL fp;
    int32_t ret;
    uint8_t *fileBuf;
    uint32_t fileSize = 0;
    uint32_t readBytes = 0;
    int len;
    int i = 0;
    const char *filePath = "1:nft.bin";
    ret = f_open(&fp, filePath, FA_OPEN_EXISTING | FA_READ);
    if (ret) {
        printf("open file err %d\n", ret);
        return;
    }

    fileSize = f_size(&fp);
    printf("fileSize = %d\n", fileSize);
    uint32_t lastLen = fileSize;
    fileBuf = SRAM_MALLOC(4096);
    fileBuf = pvPortMalloc(4096);

    while (lastLen) {
        len = lastLen > 4096 ? 4096 : lastLen;
        memset(fileBuf, 0, 4096);
        ret = f_read(&fp, (void*)fileBuf, len, &readBytes);
        if (ret) {
            FatfsError(ret);
            f_close(&fp);
            vPortFree(fileBuf);
            return;
        }
        Gd25FlashSectorErase(START_ADDR + i * 4096);
        Gd25FlashWriteBuffer(START_ADDR + i * 4096, fileBuf, len);
        i++;
        lastLen -= len;
    }
    f_close(&fp);
    f_unlink(filePath);
    SRAM_FREE(fileBuf);
}

static uint8_t *ServiceNftFileTransComplete(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    FrameHead_t sendHead = {0};
    uint8_t md5Result[16];

    ASSERT(g_fileTransTimeOutTimer);
    osTimerStop(g_fileTransTimeOutTimer);
    g_fileTransCtrl.endTick = osKernelGetTickCount();
    MD5_Final(md5Result, &ctx);
    ASSERT(memcmp(md5Result, g_fileTransInfo.md5, 16) == 0);
    printf("total tick=%d\n", g_fileTransCtrl.endTick - g_fileTransCtrl.startTick);

    sendHead.packetIndex = head->packetIndex;
    sendHead.serviceId = head->serviceId;
    sendHead.commandId = head->commandId;
    sendHead.flag.b.ack = 0;
    sendHead.flag.b.isHost = 0;

    *outLen = sizeof(FrameHead_t) + 4;
    SetNftBinValid(true);
    SaveDeviceSettings();
    WriteNftToFlash();
    GuiApiEmitSignalWithValue(SIG_INIT_NFT_BIN, 0);
    GuiApiEmitSignalWithValue(SIG_INIT_TRANSFER_NFT_SCREEN, 1);
    return BuildFrame(&sendHead, NULL, 0);
}

#endif