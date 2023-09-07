#include "simulator_model.h"
#include "librust_c.h"
#include "gui.h"
#include "gui_home_widgets.h"
#include "cjson/cJSON.h"

bool g_fingerUnlockDeviceFlag = true;
bool g_fingerSingTransitionsFlag = false;
bool fingerRegisterState[3] = {true, false, false};
#define JSON_MAX_LEN (1024 * 100)
#define ACCOUNT_PUBLIC_HOME_COIN "{\"BTC\":{\"firstRecv\":true,\"manage\":true},\"ETH\":{\"firstRecv\":true,\"manage\":true},\"BNB\":{\"firstRecv\":true,\"manage\":false},\"SOL\":{\"firstRecv\":true,\"manage\":false},\"DOT\":{\"firstRecv\":true,\"manage\":false},\"XRP\":{\"firstRecv\":true,\"manage\":false},\"LTC\":{\"firstRecv\":true,\"manage\":false},\"DASH\":{\"firstRecv\":true,\"manage\":true},\"BCH\":{\"firstRecv\":true,\"manage\":true}}"

bool g_reboot = false;
void OpenUsb()
{

}

void CloseUsb()
{
}

void SetLockDeviceAlive(bool alive)
{
    
}

int Slip39OneSliceCheck(char *wordsList, uint8_t wordCnt, uint16_t id, uint8_t ie, uint8_t *threshold)
{

}

uint16_t GetSlip39Id(void)
{

}

uint8_t GetSlip39Ie(void)
{

}

uint32_t FatfsGetSize(const char *path)
{

}

int32_t ClearCurrentPasswordErrorCount(void)
{

}

int32_t GetDevicePublicKey(uint8_t *pubkey)
{

}

struct UREncodeMultiResult *get_next_part(PtrEncoder ptr)
{

}

static uint8_t g_fpVersion[4] = {0};

uint8_t *GuiGetFpVersion(uint8_t *version)
{
    for (int i = 0; i < 4; i++) {
        version[2 * i] = g_fpVersion[i] + '0';
        if (i == 3) {
            break;
        }
        version[2 * i + 1] = '.';
    }
    return version;
}

void free_ur_encode_muilt_result(PtrT_UREncodeMultiResult ptr)
{

}

void free_ur_encode_result(PtrT_UREncodeResult ptr)
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

uint32_t GetVibration(void)
{
    return 1;
}

void SetVibration(uint32_t vibration)
{
}

void SetUSBSwitch(uint32_t usbSwitch)
{

}

uint32_t GetUSBSwitch(void)
{
    return 0;
}

void UnlimitedVibrate(int strength)
{

}

void Vibrate(int strength)
{

}

void SetAutoPowerOff(uint32_t autoPowerOff)
{

}

int32_t GetExistAccountNum(uint8_t *accountNum)
{
    *accountNum = 1;
    return SUCCESS_CODE;
}

bool SdCardInsert(void)
{
    return true;
}

uint32_t GetSetupStep(void)
{

}

void SetSetupStep(uint32_t setupStep)
{

}

bool CheckOtaBinVersion(char *version)
{
    strcpy(version, "1.1.1");
    return true;
}

void LogSetLogName(char *name)
{
}

uint32_t GetAutoLockScreen(void)
{
    return 10;
}

void SetAutoLockScreen(uint32_t autoLockScreen)
{

}

uint32_t GetAutoPowerOff(void)
{
    return 10;
}

void SetBright(uint32_t bight)
{

}

uint32_t GetBright(void)
{
    return 10;
}

void SaveDeviceSettings(void)
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

int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex)
{
    return SUCCESS_CODE;
}

uint8_t GetCurrentAccountIndex(void)
{
    return 1;
}

uint8_t GetBatterPercent(void)
{
    return 50;
}

int Slip39CheckFirstWordList(char *wordsList, uint8_t wordCnt, uint8_t *threshold)
{
    *threshold = 3;
    return SUCCESS_CODE;
}

void sha256(void *sha, const void *p, size_t size)
{
}

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
int32_t GetFpStateInfo(uint8_t *info)
{
    return 0;
}
int32_t SetFpStateInfo(uint8_t *info)
{
    return 0;
}

int32_t RegisterFp(uint8_t index)
{
    return 0;
}

int32_t FpRecognize(int type)
{
    return 0;
}

int32_t DeleteFp(uint8_t index)
{
    return 0;
}

int32_t FpCancelCurOperate(void)
{
    return 0;
}

bool GetPassphraseQuickAccess(void)
{
    return false;
}

bool PassphraseExist(uint8_t accountIndex)
{
    return false;
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

PtrT_TransactionParseResult_DisplayETHPersonalMessage eth_parse_personal_message(PtrUR ptr,
        PtrString xpub)
{
    struct TransactionParseResult_DisplayETHPersonalMessage *result = malloc(sizeof(TransactionParseResult_DisplayETHPersonalMessage));

    return result;
}

Ptr_Response_DisplayContractData eth_parse_contract_data(PtrString input_data,
        PtrString contract_json)
{
    struct Response_DisplayContractData *result = malloc(sizeof(Response_DisplayContractData));

    return result;
}

int strcasecmp(char const *a, char const* b)
{
    return 0;
}

void AccountPublicHomeCoinGet(WalletState_t *walletList, uint8_t count)
{
    lv_fs_file_t fd;
    uint32_t size = 0;
    char buf[JSON_MAX_LEN] = ACCOUNT_PUBLIC_HOME_COIN;

    if (0) {
        cJSON *rootJson, *jsonItem;
        char *retStr;
        rootJson = cJSON_CreateObject();
        for (int i = 0; i < count; i++) {
            jsonItem = cJSON_CreateObject();
            cJSON_AddItemToObject(jsonItem, "firstRecv", cJSON_CreateBool(true));
            if (!strcmp(walletList[i].name, "BTC") || !strcmp(walletList[i].name, "ETH")) {
                cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
            } else {
                cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(false));
            }
            cJSON_AddItemToObject(rootJson, walletList[i].name, jsonItem);
        }
        retStr = cJSON_Print(rootJson);
        printf("retStr = %s\n", retStr);
        // RemoveFormatChar(retStr);
        cJSON_Delete(rootJson);
    }
    cJSON *rootJson = cJSON_Parse(buf);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetObjectItem(rootJson, walletList[i].name);
        if (item != NULL) {
            cJSON *manage = cJSON_GetObjectItem(item, "manage");
            bool state = manage->valueint;
            walletList[i].state = state;
        } else {
            walletList[i].state = false;
        }
    }
}

void AccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count)
{
    lv_fs_file_t fd;
    uint32_t size = 0;
    char buf[JSON_MAX_LEN] = ACCOUNT_PUBLIC_HOME_COIN;
    cJSON *rootJson, *jsonItem;
    bool needUpdate = false;

    rootJson = cJSON_Parse(buf);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetObjectItem(rootJson, walletList[i].name);
        if (item != NULL) {
            cJSON *manage = cJSON_GetObjectItem(item, "manage");
            bool state = manage->valueint;
            if (state != walletList[i].state) {
                needUpdate = true;
                cJSON_ReplaceItemInObject(item, "manage", cJSON_CreateBool(walletList[i].state));
            }
        }
    }
}

bool GetDBContract(const char* address, const char *selector, const uint32_t chainId, char *functionABIJson, char *contractName)
{
    return false;
}

Ptr_Response_DisplayContractData eth_parse_contract_data_by_method(PtrString input_data,
        PtrString contract_name,
        PtrString contract_method_json)
{
    struct Response_DisplayContractData *result = malloc(sizeof(Response_DisplayContractData));

    return result;
}


void SystemReboot(void)
{

}

void WipeDevice(void)
{

}

void SetPageLockScreen(bool enable)
{
}

void SetFirstReceive(const char* chainName, bool isFirst)
{}

bool GetFirstReceive(const char* chainName)
{}

void SetLastLockDeviceTime(uint32_t timeStamp)
{

}

uint32_t GetCurrentStampTime(void)
{
    return 0;
}

void SetLockTimeState(bool enable)
{

}

uint32_t GetLastLockDeviceTime(void)
{
    return 0;
}

uint8_t GetLoginPasswordErrorCount(void)
{

}

void LogoutCurrentAccount(void)
{
}

void free_ur_parse_multi_result(PtrT_URParseMultiResult ptr)
{

}

void free_ur_parse_result(PtrT_URParseResult ur_parse_result)
{

}

void free_ptr_string(PtrString ptr)
{

}

PtrString calculate_auth_code(ConstPtrUR web_auth_data,
                              PtrBytes rsa_key_n,
                              uint32_t rsa_key_n_len,
                              PtrBytes rsa_key_d,
                              uint32_t rsa_key_d_len)
{

}

int32_t GetWebAuthRsaKey(uint8_t *key)
{

}

const char *GetFpErrorMessage(int errCode)
{
    return "test";
}

int32_t AsyncExecuteWithPtr(void *func, const void *inData)
{

}

void SetCurrentAccountIndex(void)
{

}