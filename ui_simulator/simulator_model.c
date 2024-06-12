#include "simulator_model.h"
#include "librust_c.h"
#include "gui.h"
#include "gui_home_widgets.h"
#include "cjson/cJSON.h"
#include "stdint.h"
#include "gui_resolve_ur.h"

#ifndef BTC_ONLY
#include "eapdu_services/service_resolve_ur.h"
#endif

bool g_fingerUnlockDeviceFlag = true;
bool g_fingerSingTransitionsFlag = false;
bool fingerRegisterState[3] = {true, false, false};
#define JSON_MAX_LEN (1024 * 100)
#define ACCOUNT_PUBLIC_HOME_COIN_PATH "C:/assets/coin.json"

bool g_reboot = false;

int32_t GetUpdatePubKey(uint8_t *pubKey)
{
    sprintf(pubKey, "%02x", 0x4);
    for (int i = 1; i < 65; i++)
    {
        sprintf(&pubKey[i], "%02x", i);
    }
}

void TrngGet(void *buf, uint32_t len)
{
    uint32_t buf4[4];
    for (uint32_t i = 0; i < len; i += 16)
    {
        for (int i = 0; i < 4; i++)
        {
            buf4[i] = 0x1 * i;
        }
        if (len - i >= 16)
        {
            memcpy((uint8_t *)buf + i, buf4, 16);
        }
        else
        {
            memcpy((uint8_t *)buf + i, buf4, len - i);
        }
    }
}

void SE_GetTRng(void *buf, uint32_t len)
{
    uint8_t *data = buf;
    for (int i = 0; i < len; i++)
    {
        uint32_t randNum = rand();
        data[i] = randNum & 0xFF;
    }
}


int32_t SE_GetDS28S60Rng(uint8_t *rngArray, uint32_t num)
{
    uint8_t *data = rngArray;
    for (int i = 0; i < num; i++)
    {
        data[i] = num - i;
    }

    return 0;
}

int32_t SE_GetAtecc608bRng(uint8_t *rngArray, uint32_t num)
{
    uint8_t *data = rngArray;
    for (int i = 0; i < num; i++)
    {
        data[i] = 2 * i;
    }

    return 0;
}

void SetLcdBright(uint32_t bright)
{
}

void SetShowPowerOffPage(bool isShow)
{
}

void FpWipeManageInfo(void)
{
}

void SetShutdownTimeOut(uint32_t timeOut)
{
}

void SetLockTimeOut(uint32_t timeOut)
{
}

bool GetUsbDetectState(void)
{
    return true;
}

bool UsbInitState(void)
{
    return true;
}

uint8_t GetExportWallet()
{
    return 1;
}

void ExportAddressReject()
{
}

void ExportAddressApprove()
{
}

bool FatfsFileExist(const char *path)
{
    lv_fs_file_t fp;
    lv_fs_res_t res = LV_FS_RES_OK;
    res = lv_fs_open(&fp, path, LV_FS_MODE_RD);
    if (res == LV_FS_RES_OK)
    {
        lv_fs_close(&fp);
        return true;
    }
    return false;
}

bool FpModuleIsChipState(void)
{
    return true;
}

int32_t InitSdCardAfterWakeup(const void *inData, uint32_t inDataLen)
{
}

bool GetLvglHandlerStatus(void)
{
}

void FpDeleteRegisterFinger(void)
{
}

void FpSaveKeyInfo(bool add)
{
}

uint16_t GetCurrentUSParsingRequestID()
{
    return 0;
}

#ifndef BTC_ONLY
void HandleURResultViaUSBFunc(const void *data, uint32_t data_len, uint16_t requestID, StatusEnum status)
{
}
#endif

uint32_t GetBatteryMilliVolt(void)
{
    return 1000;
}

bool FpModuleIsExist(void)
{
    return true;
}

void UserDelay(uint32_t ms)
{
}

void SetLockDeviceAlive(bool alive)
{
}

uint8_t *GuiGetFpVersion(char *version, uint32_t maxLen)
{
    snprintf_s(version, maxLen, "%d.%d.%d.%d", 0, 0, 0, 10);
    return version;
}

void ShowAssert(const char *file, uint32_t len)
{
    printf("assert,file=%s\r\nline=%d\r\n", file, len);
    while (1)
        ;
}

void OpenUsb()
{
}

void CloseUsb()
{
}

uint32_t FatfsGetSize(const char *path)
{
}

void UsbDeInit(void)
{
}

void UsbInit(void)
{
}

void SetUsbState(bool enable)
{
}

bool GetUsbState(void)
{
    return 1;
}

void UnlimitedVibrate(int strength)
{
}

void Vibrate(int strength)
{
}

int FormatSdFatfs(void)
{
    return 0;
}

bool SdCardInsert(void)
{
    return true;
}

void LogSetLogName(char *name)
{
}

uint32_t PubValueMsg(uint32_t messageID, uint32_t value)
{
    return 0;
}

bool Tampered(void)
{
    return false;
}

bool GetFactoryResult(void)
{
    return true;
}

size_t xPortGetFreeHeapSize(void)
{
    return 0;
}

uint8_t GetBatterPercent(void)
{
    return 60;
}

int FatfsFileWrite(const char *path, const uint8_t *data, uint32_t len)
{
    int32_t readBytes = 0;
    lv_fs_file_t fd;
    lv_fs_res_t ret = LV_FS_RES_OK;

    if (path == NULL)
    {
        return -1;
    }

    ret = lv_fs_open(&fd, path, LV_FS_MODE_WR);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }

    ret = lv_fs_write(&fd, data, len, &readBytes);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_write failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }
    lv_fs_close(&fd);

    return 0;
}

void SetLockScreen(bool enable)
{
}

void ClearLockScreenTime(void)
{
}

int32_t GetSerialNumber(char *serialNumber)
{
    strcpy(serialNumber, "DVT230712345");
    return 0;
}

bool IsPreviousLockScreenEnable(void)
{
    return true;
}

int32_t RegisterFp(uint8_t index)
{
    return 0;
}

int32_t FpRecognize(int type)
{
    return 0;
}

void DeleteFp(uint8_t index)
{
}

int32_t FpCancelCurOperate(void)
{
    return 0;
}

uint8_t GetFingerSignFlag(void)
{
    return 1;
}

void UpdateFingerUnlockFlag(uint8_t unlockFlag)
{
}

void SetFingerManagerInfoToSE(void)
{
}

void UpdateFingerSignFlag(uint8_t index, bool signFlag)
{
}

uint8_t GetFingerRegisteredStatus(uint8_t fingerIndex)
{
    return 1;
}

uint8_t GetRegisteredFingerNum(void)
{
    return 0;
}

uint8_t GetFingerUnlockFlag(void)
{
    return 1;
}

bool GetDBContract(const char *address, const char *selector, const uint32_t chainId, char *functionABIJson, char *contractName)
{
    return false;
}

void SystemReboot(void)
{
}

void SetPageLockScreen(bool enable)
{
}

uint32_t GetCurrentStampTime(void)
{
    return 0;
}

void SetLockTimeState(bool enable)
{
}

int32_t GetWebAuthRsaKey(uint8_t *key)
{
}

int32_t AsyncExecuteWithPtr(void *func, const void *inData)
{
}

static uint8_t buffer[100 * 1024];

static char *qrcode[100];
static uint32_t qrcode_size;

char *FatfsFileRead(const char *path)
{
    int32_t readBytes = 0;
    lv_fs_file_t fd;
    lv_fs_res_t ret = LV_FS_RES_OK;
    char *buffer = SRAM_MALLOC(1024 * 100);
    char truePath[64] = "C:/assets/sd/";
    strcat(truePath, path);
    printf("truePath: %s\n", truePath);

    ret = lv_fs_open(&fd, truePath, LV_FS_MODE_RD);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return NULL;
    }

    ret = lv_fs_read(&fd, buffer, 1024 * 100, &readBytes);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_read failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return NULL;
    }
    printf("readBytes: %d\n", readBytes);
    lv_fs_close(&fd);

    return buffer;
}

uint8_t *FatfsFileReadBytes(const char *path, uint32_t *readBytes)
{
    lv_fs_file_t fd;
    lv_fs_res_t ret = LV_FS_RES_OK;
    uint8_t *buffer = SRAM_MALLOC(1024 * 100);
    char truePath[64] = "C:/assets/sd/";
    strcat(truePath, path);
    printf("truePath: %s\n", truePath);

    ret = lv_fs_open(&fd, truePath, LV_FS_MODE_RD);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return NULL;
    }

    ret = lv_fs_read(&fd, buffer, 1024 * 100, readBytes);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_read failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return NULL;
    }
    printf("readBytes: %u\n", *readBytes);
    lv_fs_close(&fd);

    return buffer;
}

int32_t prepare_qrcode()
{
    printf("prepare_qrcode\r\n");
    char *path = "C:/assets/qrcode_data.txt";
    memset(buffer, '\0', 100 * 1024);

    lv_fs_file_t fd;
    lv_fs_res_t ret = LV_FS_RES_OK;
    ret = lv_fs_open(&fd, path, LV_FS_MODE_RD);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }

    int32_t readBytes = 0;

    ret = lv_fs_read(&fd, buffer, 100 * 1024, &readBytes);
    if (ret != LV_FS_RES_OK)
    {
        printf("lv_fs_read failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }
    lv_fs_close(&fd);

    int lastIndex = 0;
    int lastQRIndex = 0;

    for (size_t i = 0; i < readBytes; i++)
    {
        if (buffer[i] == '\n')
        {
            if (qrcode[lastQRIndex] != NULL)
            {
                free(qrcode[lastQRIndex]);
            }
            qrcode[lastQRIndex] = malloc(1024);
            memset(qrcode[lastQRIndex], '\0', 1024);
            memcpy(qrcode[lastQRIndex], buffer + lastIndex, i - lastIndex);
            printf("qrcode: %s\r\n", qrcode[lastQRIndex]);
            lastIndex = i + 1;
            lastQRIndex++;
        }
        if (i == readBytes - 1)
        {
            printf("last char: %c\r\n", buffer[i]);
            if (qrcode[lastQRIndex] != NULL)
            {
                free(qrcode[lastQRIndex]);
            }
            qrcode[lastQRIndex] = malloc(1024);
            memset(qrcode[lastQRIndex], '\0', 1024);
            memcpy(qrcode[lastQRIndex], buffer + lastIndex, i - lastIndex + 1);
            printf("qrcode: %s\r\n", qrcode[lastQRIndex]);
            qrcode_size = lastQRIndex + 1;
        }
    }
    printf("read: %d\r\n", readBytes);

    return readBytes;
}

int32_t read_qrcode()
{
    UrViewType_t viewType;
    uint32_t readLen = prepare_qrcode();
    if (readLen == 0)
        return 0;
    int i = 0;
    int loopTime = 0;

    struct URParseResult *urResult;
    bool firstQrFlag = true;
    PtrDecoder decoder = NULL;

    char *qrString = qrcode[i++];
    printf("qrString read: %s\r\n", qrString);
    urResult = parse_ur(qrString);
    if (urResult->error_code == 0)
    {
        if (urResult->is_multi_part == 0)
        {
            // single qr code
            firstQrFlag = true;
            viewType.viewType = urResult->t;
            viewType.urType = urResult->ur_type;
            handleURResult(urResult, NULL, viewType, false);
            return 0;
        }
        else
        {
            // first qr code
            firstQrFlag = false;
            decoder = urResult->decoder;
        }
    }

    printf("qrcode_size: %d\r\n", qrcode_size);

    while (1)
    {
        if (loopTime++ >= 1000)
        {
            printf("qrcode decode loop time exceeded\r\n");
            break;
        }
        i = i % qrcode_size;

        qrString = qrcode[i];
        printf("qrString read: %s\r\n", qrString);

        // follow qrcode
        struct URParseMultiResult *MultiurResult = receive(qrString, decoder);
        if (MultiurResult->error_code == 0)
        {
            if (MultiurResult->is_complete)
            {
                firstQrFlag = true;
                viewType.viewType = MultiurResult->t;
                viewType.urType = MultiurResult->ur_type;
                handleURResult(NULL, MultiurResult, viewType, true);
                return 0;
            }
        }
        else
        {
            printf("error code: %d\r\n", MultiurResult->error_code);
            printf("error message: %s\r\n", MultiurResult->error_message);
            break;
        }
        if (!(MultiurResult->is_complete))
        {
            free_ur_parse_multi_result(MultiurResult);
        }

        i++;
    }
}

bool GetEnsName(const char *addr, char *name) {
    return false;
}