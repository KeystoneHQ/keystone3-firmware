#include "simulator_model.h"
#include "librust_c.h"
#include "gui.h"
#include "gui_home_widgets.h"
#include "cjson/cJSON.h"
#ifndef BTC_ONLY
#include "eapdu_services/service_resolve_ur.h"
#endif

bool g_fingerUnlockDeviceFlag = true;
bool g_fingerSingTransitionsFlag = false;
bool fingerRegisterState[3] = {true, false, false};
#define JSON_MAX_LEN (1024 * 100)
#define ACCOUNT_PUBLIC_HOME_COIN_PATH "C:/assets/coin.json"

bool g_reboot = false;

void TrngGet(void *buf, uint32_t len)
{
    uint32_t buf4[4];
    for (uint32_t i = 0; i < len; i += 16) {
        for (int i = 0; i < 4; i++) {
            buf4[i] = 0x1 * i;
        }
        if (len - i >= 16) {
            memcpy((uint8_t *)buf + i, buf4, 16);
        } else {
            memcpy((uint8_t *)buf + i, buf4, len - i);
        }
    }
}

void SE_GetTRng(void *buf, uint32_t len)
{
    uint8_t *data = buf;
    for (int i = 0; i < len; i++) {
        uint32_t randNum = rand();
        data[i] = randNum & 0xFF;
    }
}

int32_t SE_GetDS28S60Rng(uint8_t *rngArray, uint32_t num)
{
    uint8_t *data = rngArray;
    for (int i = 0; i < num; i++) {
        data[i] = num - i;
    }

    return 0;
}

int32_t SE_GetAtecc608bRng(uint8_t *rngArray, uint32_t num)
{
    uint8_t *data = rngArray;
    for (int i = 0; i < num; i++) {
        data[i] = 2 * i;
    }
    
    return 0;
}

PtrT_UREncodeResult get_connect_companion_app_ur(PtrBytes master_fingerprint,
                                                 uint32_t master_fingerprint_length,
                                                 int32_t cold_version,
                                                 PtrT_CoinConfig coin_config,
                                                 uint32_t coin_config_length)
{

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
    return true;
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

}

#ifndef BTC_ONLY
PtrT_TransactionParseResult_EthParsedErc20Transaction eth_parse_erc20(PtrString input,
                                                                      uint32_t decimal)
{

}
struct UREncodeResult *get_connect_metamask_ur_unlimited(PtrBytes master_fingerprint,
                                                         uint32_t master_fingerprint_length,
                                                         enum ETHAccountType account_type,
                                                         PtrT_CSliceFFI_ExtendedPublicKey public_keys)
{

}

struct UREncodeResult *get_connect_metamask_ur(PtrBytes master_fingerprint,
                                               uint32_t master_fingerprint_length,
                                               enum ETHAccountType account_type,
                                               PtrT_CSliceFFI_ExtendedPublicKey public_keys)
{

}
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

uint8_t *GuiGetFpVersion(char *version)
{
    for (int i = 0; i < 4; i++) {
        version[2 * i] = 1 + '0';
        if (i == 3) {
            break;
        }
        version[2 * i + 1] = '.';
    }
    return version;
}

void ShowAssert(const char* file, uint32_t len)
{
    printf("assert,file=%s\r\nline=%d\r\n", file, len);
    while (1);
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

bool CheckOtaBinVersion(char *version)
{
    strcpy(version, "1.1.1");
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
    return 50;
}

int FatfsFileWrite(const char* path, const uint8_t *data, uint32_t len){}
int FormatSdFatfs(void){}


void SetLockScreen(bool enable)
{
}

int32_t GetSerialNumber(char* serialNumber)
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

bool GetDBContract(const char* address, const char *selector, const uint32_t chainId, char *functionABIJson, char *contractName)
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
