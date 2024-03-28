/* INCLUDES */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "assert.h"
#include "define.h"
#include "mhscpu.h"
#include "drv_uart.h"
#include "drv_motor.h"
#include "drv_trng.h"
#include "user_memory.h"
#include "log_print.h"
#include "ctaes.h"
#include "keystore.h"
#include "account_manager.h"
#include "gui_views.h"
#include "gui_api.h"
#include "secret_cache.h"
#include "user_msg.h"
#include "user_utils.h"
#include "user_fatfs.h"
#include "secret_cache.h"
#include "fingerprint_process.h"
#include "fingerprint_crc.h"
#include "fingerprint_task.h"
#include "event_groups.h"
#include "user_delay.h"
#include "screen_manager.h"
#include "low_power.h"
#include "se_manager.h"

/* DEFINES */
#define FINGERPRINT_REG_MAX_TIMES               (18)

/* TYPEDEFS */

/* FUNC DECLARATION*/
static uint8_t *GetRandomAesKey(uint8_t *hostRandomKey, uint8_t *fpRandomKey, uint8_t *communicateAesKey);
static int SendDataToFp(uint8_t *pBuffer, int size);
static void EncryptFingerprintData(uint8_t *cipherData, const uint8_t *plainData, uint16_t len);
static void FpGenericSend(uint16_t cmd, uint8_t isEncrypt);
static void FpGenericRecv(char *indata, uint8_t len);
static void FpUpdateUpdateRandom(char *indata, uint8_t len);
static void FpGetChipId(char *indata, uint8_t len);
static void FpGetVersion(char *indata, uint8_t len);
static void FpRegSend(uint16_t cmd, uint8_t param);
static void FpRegRecv(char *indata, uint8_t len);
static void FpDeleteSend(uint16_t cmd, uint8_t index);
static void FpDeleteRecv(char *indata, uint8_t len);
static void FpGetNumberSend(uint16_t cmd, uint8_t fingerInfo);
static void FpGetNumberRecv(char *indata, uint8_t len);
static void FpRecognizeSend(uint16_t cmd, uint8_t passwd);
static void FpRecognizeRecv(char *indata, uint8_t len);
static void FpGetUid(char *indata, uint8_t len);
static void FpCancelRecv(char *indata, uint8_t len);
static void FpGetAesKeyState(char *indata, uint8_t len);
static void FpGetInitStateRecv(char *indata, uint8_t len);
static void DeleteFingerManager(uint8_t index);
static void AddFingerManager(uint8_t index);
static void FpResponseHandle(uint16_t cmd);
static void FpLowerPowerRecv(char *indata, uint8_t len);
static void FpLowerPowerSend(uint16_t cmd, uint8_t param);
static void FpSetAesKeySend(uint16_t cmd, uint8_t fingerInfo);
static void FpSetAesKeyRecv(char *indata, uint8_t len);
static void FpDeleteAllRecv(char *indata, uint8_t len);
static void FpDelayMsgSend(void);
static void SearchFpAesKeyState(void);
static void SearchFpChipId(void);
bool GuiLockScreenIsTop(void);

/* STATIC VARIABLES */
extern osTimerId_t g_fpTimeoutTimer;
static osMutexId_t g_fpResponseMutex;
EventGroupHandle_t g_fpEventGroup = NULL;
static osTimerId_t g_delayCmdTimer = NULL;
static bool g_fpStatusExist = false;
static bool g_delFpSave = false;
static uint8_t g_fpIndex;
static uint16_t g_delayCmd;
static uint16_t g_lastCmd;
static bool g_devLogSwitch = false;
static uint8_t g_fpRandomKey[16] = {0};
static uint8_t g_hostRandomKey[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10};
static uint8_t g_communicateAesKey[32] = {0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};
static uint8_t g_randomAesKey[16] = {0};
static uint8_t g_fpTempAesKey[32] = {0};
static Recognize_Type g_fingerRecognizeType = RECOGNIZE_UNLOCK;
static uint8_t g_fpRegCnt = 0;
static FingerManagerInfo_t g_fpManager = {0};
static uint8_t g_fpVersion[4] = {0};
static bool g_chipIdState = false;
char g_intrRecvBuffer[RCV_MSG_MAX_LEN] = {0};

static const FingerPrintControl_t g_cmdHandleMap[] = {
    {FINGERPRINT_CMD_GET_RANDOM_NUM,    false,  FpGenericSend,      FpUpdateUpdateRandom},
    {FINGERPRINT_CMD_REG,               true,   FpRegSend,          FpRegRecv},
    {FINGERPRINT_CMD_DELETE_SINGLE,     true,   FpDeleteSend,       FpDeleteRecv},
    {FINGERPRINT_CMD_DELETE_ALL,        true,   FpDeleteSend,       FpDeleteAllRecv},
    {FINGERPRINT_CMD_GET_REG_NUM,       true,   FpGetNumberSend,    FpGetNumberRecv},
    {FINGERPRINT_CMD_RECOGNIZE,         true,   FpRecognizeSend,    FpRecognizeRecv},
    {FINGERPRINT_CMD_GET_CHIP_ID,       true,   FpGenericSend,      FpGetChipId},
#ifdef ENABLE_FP_RESET
    {FINGERPRINT_CMD_SYS_RESET,         true,   FpGenericSend,      FpGenericRecv},
#endif
    {FINGERPRINT_CMD_GET_UID,           true,   FpGenericSend,      FpGetUid},
    {FINGERPRINT_CMD_GET_VER,           false,  FpGenericSend,      FpGetVersion},
    {FINGERPRINT_CMD_CANCEL_EXECUTE,    true,   FpGenericSend,      FpCancelRecv},
    {FINGERPRINT_CMD_GET_INIT_STATE,    true,   FpGenericSend,      FpGetInitStateRecv},
    {FINGERPRINT_CMD_SET_AES_KEY,       false,  FpSetAesKeySend,    FpSetAesKeyRecv},
    {FINGERPRINT_CMD_GET_AES_KEY_STATE, false,  FpGenericSend,      FpGetAesKeyState},
    {FINGERPRINT_CMD_LOW_POWER,         true,   FpLowerPowerSend,   FpLowerPowerRecv},
};

static FingerPrintTimeout_t g_cmdTimeoutMap[] = {
    {FINGERPRINT_CMD_GET_RANDOM_NUM,        FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_REG,                   FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_DELETE_SINGLE,         FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_DELETE_ALL,            FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_GET_REG_NUM,           FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_RECOGNIZE,             FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_GET_CHIP_ID,           FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
#ifdef ENABLE_FP_RESET
    {FINGERPRINT_CMD_SYS_RESET,             FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
#endif
    {FINGERPRINT_CMD_GET_UID,               FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_GET_VER,               FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_CANCEL_EXECUTE,        FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_GET_INIT_STATE,        FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_SET_AES_KEY,           FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_GET_AES_KEY_STATE,     FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_LOW_POWER,             FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
};

/* FUNC */
static void FpDelayTimerHandle(void *argument)
{
    FpDelayMsgSend();
}

bool FpModuleIsExist(void)
{
    return g_fpStatusExist;
}

bool FpModuleIsChipState(void)
{
    return g_chipIdState;
}

void FpSendTimerStart(uint16_t cmd)
{
    for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(g_cmdTimeoutMap); i++) {
        if (g_cmdTimeoutMap[i].cmd == cmd) {
            g_cmdTimeoutMap[i].cnt = 0;
        }
    }
    if (osTimerIsRunning(g_fpTimeoutTimer) == 0) {
        osTimerStart(g_fpTimeoutTimer, 100);
    }
}

// A000 SEND
static void FpRegSend(uint16_t cmd, uint8_t param)
{
    UNUSED(cmd);
    g_fpRegCnt = 0;
    g_delayCmd = FINGERPRINT_CMD_REG;
    g_fpIndex = param;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    FpSendTimerStart(FINGERPRINT_CMD_REG);
    // SendPackFingerMsg(FINGERPRINT_CMD_REG, &param, 0, 1, AES_KEY_ENCRYPTION);
}

void FpDeleteRegisterFinger(void)
{
    g_delFpSave = true;
    DeleteFp(g_fpIndex);
}

// A000 RECV
void FpRegRecv(char *indata, uint8_t len)
{
    int i = 0;
    uint8_t result = indata[i++];
    g_fpRegCnt++;
    ClearLockScreenTime();
    if (result == FP_SUCCESS_CODE) {
        g_fpIndex = indata[i++];
        uint8_t cnt = indata[i];
        printf("finger index = %d\n", indata[i]);
        MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_SHORT_TIME);
        if (len == 38) {
            printf("save account index = %d\n", GetCurrentAccountIndex());
            memcpy_s(g_fpTempAesKey, sizeof(g_fpTempAesKey), (uint8_t *)&indata[6], sizeof(g_fpTempAesKey));
        }
        GuiApiEmitSignal(SIG_FINGER_REGISTER_STEP_SUCCESS, &cnt, sizeof(cnt));
    } else {
        if (g_fpRegCnt == FINGERPRINT_REG_MAX_TIMES) {
            MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_ULTRA_LONG_TIME);
            FpCancelCurOperate();
            GuiApiEmitSignal(SIG_FINGER_REGISTER_EXCEEDED_TIMES, &result, sizeof(result));
        } else {
            MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_LONG_TIME);
            GuiApiEmitSignal(SIG_FINGER_REGISTER_STEP_FAIL, &result, sizeof(result));
        }
    }

    printf("\n");
}

// A400 SEND
// index
static void FpDeleteSend(uint16_t cmd, uint8_t index)
{
    g_delayCmd = cmd;
    g_fpIndex = index;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    FpSendTimerStart(cmd);
    // SendPackFingerMsg(cmd, &index, 0, 1, AES_KEY_ENCRYPTION);
}

// A400 RECV
static void FpDeleteRecv(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        if (g_delFpSave == true) {
            g_delFpSave = false;
            GuiApiEmitSignal(SIG_FINGER_DELETE_SUCCESS, NULL, 0);
            return;
        }
        printf("delete success %d\n", g_fpIndex);
        DeleteFingerManager(g_fpIndex);
        if (g_fpManager.fingerNum == 0) {
            memset_s(&g_fpManager, sizeof(g_fpManager), 0, sizeof(g_fpManager));
        }
        SetFingerManagerInfoToSE();
        GuiApiEmitSignal(SIG_FINGER_DELETE_SUCCESS, NULL, 0);
    }
}

void FpDeleteAllRecv(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        printf("delete success\n");
        memset_s(&g_fpManager, sizeof(g_fpManager), 0, sizeof(g_fpManager));
    }
}

// A500 SEND
// fingerInfo 0 only get num 1 get info
static void FpGetNumberSend(uint16_t cmd, uint8_t fingerInfo)
{
    g_delayCmd = FINGERPRINT_CMD_GET_REG_NUM;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    FpSendTimerStart(cmd);
}

// A500 RECV
static void FpGetNumberRecv(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        FpResponseHandle(FINGERPRINT_CMD_GET_REG_NUM);
        printf("fingerprints currently exist %d\n", indata[1]);
        GuiApiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
        if (g_fpManager.fingerNum == 0) {
            memset_s(&g_fpManager, sizeof(g_fpManager), 0, sizeof(g_fpManager));
        }
        if (indata[1] != g_fpManager.fingerNum) {
            memset_s(&g_fpManager, sizeof(g_fpManager), 0, sizeof(g_fpManager));
            SetFingerManagerInfoToSE();
            FpDeleteSend(FINGERPRINT_CMD_DELETE_ALL, 1);
        }
    }
    if (g_chipIdState == false) {
        SearchFpChipId();
    }
}

// A7FF SEND
// passwd 0 not need passwd 1 need passwd
static void FpRecognizeSend(uint16_t cmd, uint8_t passwd)
{
    g_delayCmd = FINGERPRINT_CMD_RECOGNIZE;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    FpSendTimerStart(FINGERPRINT_CMD_RECOGNIZE);
    // SendPackFingerMsg(FINGERPRINT_CMD_RECOGNIZE, &passwd, 0, 1, AES_KEY_ENCRYPTION);
}

void FpSaveKeyInfo(bool add)
{
    if (add) {
        AddFingerManager(g_fpIndex);
    }
    SetFingerManagerInfoToSE();
    FingerSetInfoToSE(g_fpTempAesKey, 0, GetCurrentAccountIndex(), SecretCacheGetPassword());
    memset_s(g_fpTempAesKey, sizeof(g_fpTempAesKey), 0,  sizeof(g_fpTempAesKey));
    ClearSecretCache();
}

// A7FF RECV
static void FpRecognizeRecv(char *indata, uint8_t len)
{
    int i = 0;
    uint8_t result = indata[i++];
    ClearLockScreenTime();
    if (result == FP_SUCCESS_CODE) {
        MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_SHORT_TIME);
        printf("recognize score is %d\n", indata[i++]);
        if (len == 38) {
            if (g_fingerRecognizeType == RECOGNIZE_OPEN_SIGN) {
                if (GetCurrentAccountIndex() != 0xFF) {
                    memcpy_s(g_fpTempAesKey, sizeof(g_fpTempAesKey), (uint8_t *)&indata[6], sizeof(g_fpTempAesKey));
                    FpSaveKeyInfo(false);
                } else {
                    return;
                }
            } else {
                uint8_t encryptedPassword[32] = {0};
                char decryptPasscode[128] = {0};
                GetFpEncryptedPassword(GetCurrentAccountIndex(), encryptedPassword);
                decryptFunc((uint8_t *)decryptPasscode, encryptedPassword, (uint8_t *)&indata[6]);
                SecretCacheSetPassword(decryptPasscode);
                memset_s(decryptPasscode, sizeof(decryptPasscode), 0, sizeof(decryptPasscode));
                memset_s(encryptedPassword, sizeof(encryptedPassword), 0, sizeof(encryptedPassword));
            }
        }

        if (GuiLockScreenIsTop()) {
            if (g_fpManager.unlockFlag) {
                GuiApiEmitSignal(SIG_VERIFY_FINGER_PASS, NULL, 0);
            }
        } else {
            GuiApiEmitSignal(SIG_FINGER_RECOGNIZE_RESPONSE, &result, sizeof(result));
        }
    } else {
        MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_LONG_TIME);
        GetFpErrorMessage(result);
        printf("recognize failed\n");
        if (GuiLockScreenIsTop() && g_fpManager.unlockFlag) {
            GuiApiEmitSignal(SIG_VERIFY_FINGER_FAIL, NULL, 0);
        } else {
            GuiApiEmitSignal(SIG_FINGER_RECOGNIZE_RESPONSE, &result, sizeof(result));
        }
    }
}

// D000 SEND
static void FpSetAesKeySend(uint16_t cmd, uint8_t fingerInfo)
{
    memset_s(g_communicateAesKey, sizeof(g_communicateAesKey), 0, sizeof(g_communicateAesKey));
    TrngGet(g_communicateAesKey, sizeof(g_communicateAesKey));
    PrintArray("g_communicateAesKey", g_communicateAesKey, 32);
    SendPackFingerMsg(FINGERPRINT_CMD_SET_AES_KEY, g_communicateAesKey, 0, sizeof(g_communicateAesKey), NO_ENCRYPTION);
    FpSendTimerStart(FINGERPRINT_CMD_SET_AES_KEY);
}

// D000 RECV
void FpSetAesKeyRecv(char *indata, uint8_t len)
{
    if (len == 1) {
        uint8_t result = indata[0];
        if (result == FP_SUCCESS_CODE) {
            PrintArray("g_communicateAesKey", g_communicateAesKey, 32);
            SetFpCommAesKey(g_communicateAesKey);
            printf("set aes key success\n");
            SearchFpChipId();
        } else {
            printf("set aes key failed\n");
            GetFpErrorMessage(result);
        }
    } else if (len == 0) {
        FpResponseHandle(FINGERPRINT_CMD_SET_AES_KEY);
    }
}

void FpGenericSend(uint16_t cmd, uint8_t isEncrypt)
{
    uint8_t reserveData = 0;
    if (isEncrypt == 1) {
        g_delayCmd = cmd;
        FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
        return;
    }
    SendPackFingerMsg(cmd, &reserveData, 0, 1, isEncrypt);
    FpSendTimerStart(cmd);
}

void FpGenericRecv(char *indata, uint8_t len)
{
    printf("len = %d\n", len);
    for (int i = 0; i < len; i++) {
        printf("%02X ", indata[i]);
    }
    printf("\n");
}

uint8_t *GuiGetFpVersion(char *version)
{
    for (int i = 0; i < 4; i++) {
        version[2 * i] = g_fpVersion[i] + '0';
        if (i == 3) {
            break;
        }
        version[2 * i + 1] = '.';
    }
    return (uint8_t *)version;
}

static void FpGetVersion(char *indata, uint8_t len)
{
    static bool firstIn = true;
    if (firstIn) {
        SearchFpAesKeyState();
        firstIn = false;
        g_fpStatusExist = true;
    }
    printf("version:%d.%d.%d.%d\n", indata[0], indata[1], indata[2], indata[3]);
    memcpy_s(g_fpVersion, sizeof(g_fpVersion), indata, sizeof(g_fpVersion));
}

static void FpGetAesKeyState(char *indata, uint8_t len)
{
    uint8_t state = 0;
    uint8_t fpAesState = 0;

    if (len == 0) {
        FpResponseHandle(FINGERPRINT_CMD_GET_AES_KEY_STATE);
    } else {
        printf("aes key %s\n", indata[0] ? "has been set" : "not set");
        state = indata[0];
        fpAesState = FpAesKeyExist();
        assert(!!state == !!fpAesState);
        if (fpAesState == 0) {
            FpSetAesKeySend(0, 0);
        } else {
            InitFingerManagerInfo();
            printf("unlockFlag = %d\n", g_fpManager.unlockFlag);
            printf("signFlag[0] = %d\n", g_fpManager.signFlag[0]);
            printf("signFlag[1] = %d\n", g_fpManager.signFlag[1]);
            printf("signFlag[2] = %d\n", g_fpManager.signFlag[2]);
            printf("fingerNum = %d\n", g_fpManager.fingerNum);
            printf("fingerId[0] = %d\n", g_fpManager.fingerId[0]);
            printf("fingerId[1] = %d\n", g_fpManager.fingerId[1]);
            printf("fingerId[2] = %d\n", g_fpManager.fingerId[2]);
            FpGetNumberSend(FINGERPRINT_CMD_GET_REG_NUM, 0);
        }
    }
}

static void FpDelayMsgSend(void)
{
    g_lastCmd = g_delayCmd;
    uint8_t reserveData = 0;
    if (g_delayCmd == FINGERPRINT_CMD_RECOGNIZE) {
        if (g_fingerRecognizeType >= 1) {
            reserveData = 1;
        }
        SendPackFingerMsg(FINGERPRINT_CMD_RECOGNIZE, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_RECOGNIZE);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_LOW_POWER) {
        uint8_t passwd = 0;
        SendPackFingerMsg(FINGERPRINT_CMD_LOW_POWER, &passwd, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_LOW_POWER);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_GET_REG_NUM) {
        uint8_t reserveData = 0;
        SendPackFingerMsg(FINGERPRINT_CMD_GET_REG_NUM, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_GET_REG_NUM);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_DELETE_ALL) {
        SendPackFingerMsg(FINGERPRINT_CMD_DELETE_ALL, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_DELETE_ALL);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_DELETE_SINGLE) {
        SendPackFingerMsg(FINGERPRINT_CMD_DELETE_SINGLE, &g_fpIndex, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_DELETE_SINGLE);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_REG) {
        SendPackFingerMsg(FINGERPRINT_CMD_REG, &g_fpIndex, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_REG);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_GET_CHIP_ID) {
        SendPackFingerMsg(FINGERPRINT_CMD_GET_CHIP_ID, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_GET_CHIP_ID);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_CANCEL_EXECUTE) {
        SendPackFingerMsg(FINGERPRINT_CMD_CANCEL_EXECUTE, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_CANCEL_EXECUTE);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_GET_INIT_STATE) {
        SendPackFingerMsg(FINGERPRINT_CMD_GET_INIT_STATE, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_GET_INIT_STATE);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_GET_UID) {
        SendPackFingerMsg(FINGERPRINT_CMD_GET_UID, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_GET_UID);
        g_delayCmd = 0;
    }
}

static void FpUpdateUpdateRandom(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        memcpy_s(g_fpRandomKey, sizeof(g_fpRandomKey), &indata[1], 16);
        FpDelayMsgSend();
    } else {
        printf("update random failed\n");
        GetFpErrorMessage(result);
    }
}

static void FpGetChipId(char *indata, uint8_t len)
{
    g_chipIdState = true;
    printf("chip id is :");
    for (int i = 0; i < len; i++) {
        printf("%X", indata[i]);
    }
    printf("\n");
}

static void FpGetUid(char *indata, uint8_t len)
{
    printf("uid is :");
    int i = 0;
    uint8_t result = indata[i++];
    if (result == FP_SUCCESS_CODE) {
        for (; i < len; i++) {
            printf("%X", indata[i]);
        }
        printf("\n");
    } else {
        GetFpErrorMessage(result);
    }
}

static void FpCancelRecv(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        for (int i = 0; i < len; i++) {
            printf("%X", indata[i]);
        }
        printf("\n");
        printf("");
    } else {
        GetFpErrorMessage(result);
    }
}

static void FpGetInitStateRecv(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        printf("finger printf module has Bind\n");
    } else {
        printf("finger printf module has not bind\n");
        GetFpErrorMessage(result);
        // todo need to bind
    }
}

void FpLowerPowerSend(uint16_t cmd, uint8_t param)
{
    g_delayCmd = FINGERPRINT_CMD_LOW_POWER;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
}

void FpLowerPowerRecv(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    printf("result = %d\n", result);
    if (len == 0) {
        // FpResponseHandleStop();
    } else {
        // LowerPowerTimerStart();
    }
}

void PrintMsg(FpEncryptData_t *msg)
{
    printf("msg: cmd = 0x%02X%02X, frameIndex = %d, dataLen = %d\n", msg->cmd0, msg->cmd1, msg->frameNum, msg->dataLen);
}

void InitFingerManagerInfo(void)
{
    int32_t ret = GetFpStateInfo((uint8_t *)&g_fpManager);
    PRINT_ERRCODE(ret);
    assert(ret == 0);
}

void FingerManagerPrint(void)
{
    printf("unlockFlag = %d\n", g_fpManager.unlockFlag);
    printf("signFlag[0] = %d\n", g_fpManager.signFlag[0]);
    printf("signFlag[1] = %d\n", g_fpManager.signFlag[1]);
    printf("signFlag[2] = %d\n", g_fpManager.signFlag[2]);
    printf("fingerNum = %d\n", g_fpManager.fingerNum);
    printf("fingerId[0] = %d\n", g_fpManager.fingerId[0]);
    printf("fingerId[1] = %d\n", g_fpManager.fingerId[1]);
    printf("fingerId[2] = %d\n", g_fpManager.fingerId[2]);
}

static void AddFingerManager(uint8_t index)
{
    for (int i = 0; i < 3; i++) {
        if (i != GetCurrentAccountIndex()) {
            g_fpManager.signFlag[i] = false;
        }
    }
    if (g_fpManager.fingerNum == 0) {
        g_fpManager.unlockFlag = true;
    }
    g_fpManager.fingerNum++;
    g_fpManager.fingerId[index] = index + 1;
}

static void DeleteFingerManager(uint8_t index)
{
    if (g_fpManager.fingerNum > 0) {
        g_fpManager.fingerNum--;
    }
    g_fpManager.fingerId[index] = 0;
}

void SetFingerManagerInfoToSE(void)
{
    int32_t ret = SetFpStateInfo((uint8_t *)&g_fpManager);
    PRINT_ERRCODE(ret);
    assert(ret == 0);
}

uint8_t GetFingerUnlockFlag(void)
{
    return g_fpManager.unlockFlag;
}

void UpdateFingerUnlockFlag(uint8_t unlockFlag)
{
    g_fpManager.unlockFlag = unlockFlag;
    SetFingerManagerInfoToSE();
}

uint8_t GetFingerSignFlag(void)
{
    return g_fpManager.signFlag[GetCurrentAccountIndex()];
}

void UpdateFingerSignFlag(uint8_t index, bool signFlag)
{
    g_fpManager.signFlag[index] = signFlag;
    SetFingerManagerInfoToSE();
}

uint8_t GetRegisteredFingerNum(void)
{
    return g_fpManager.fingerNum;
}

void UpdateRegisteredFingerNum(uint8_t num)
{
    g_fpManager.fingerNum = num;
}

uint8_t GetFingerRegisteredStatus(uint8_t fingerIndex)
{
    return g_fpManager.fingerId[fingerIndex];
}

void UpdateFingerRegisteredStatus(uint8_t fingerIndex, uint8_t data)
{
    g_fpManager.fingerId[fingerIndex] = data;
}

void FingerSetInfoToSE(uint8_t *random, uint8_t fingerId, uint8_t accountIndex, char *password)
{
    uint8_t cipherData[32] = {0};
    uint8_t plainData[128] = {0};

    strcpy_s((char *)plainData, PASSWORD_MAX_LEN, password);
    AES256_ctx ctx;
    AES256_init(&ctx, random);
    AES256_encrypt(&ctx, 2, cipherData, plainData);
    int32_t ret = SetFpEncryptedPassword(accountIndex, cipherData);
    PRINT_ERRCODE(ret);
    assert(ret == 0);
}

void UpdateHostRandom(void)
{
    TrngGet(g_hostRandomKey, 16);
}

static uint8_t *GetRandomAesKey(uint8_t *hostRandomKey, uint8_t *fpRandomKey, uint8_t *communicateAesKey)
{
    if (communicateAesKey == NULL) {
        memset_s(g_randomAesKey, 16, 0, 16);
    } else {
        AES128_ctx ctx1;
        uint8_t ptr[16] = {0};
        for (int i = 0; i < 16; i++) {
            ptr[i] = hostRandomKey[i] ^ fpRandomKey[i];
        }
        AES128_init(&ctx1, &communicateAesKey[0]);
        AES128_encrypt(&ctx1, 1, g_randomAesKey, ptr);
    }
    return g_randomAesKey;
}

static void EncryptFingerprintData(uint8_t *cipherData, const uint8_t *plainData, uint16_t len)
{
    AES128_ctx ctx;
    AES128_init(&ctx, g_hostRandomKey);

    AES128_encrypt(&ctx, len / 16, cipherData, plainData);
}

static void DecryptFingerprintData(uint8_t *decipheredData, const uint8_t *cipherData, uint16_t len)
{
    AES128_ctx ctx;
    AES128_init(&ctx, g_hostRandomKey);
    AES128_decrypt(&ctx, len / 16, decipheredData, cipherData);
}

void RegisterFp(uint8_t index)
{
    printf("%s reg finger id is %d\n", __func__, index);
    FpRegSend(FINGERPRINT_CMD_REG, index);
}

void DeleteFp(uint8_t index)
{
    if (index == 0xff) {
        FpDeleteSend(FINGERPRINT_CMD_DELETE_ALL, 1);
    } else {
        FpDeleteSend(FINGERPRINT_CMD_DELETE_SINGLE, index);
    }
}

void SearchFpNum(void)
{
    FpGetNumberSend(FINGERPRINT_CMD_GET_REG_NUM, 0);
}

void FpRecognize(Recognize_Type type)
{
    uint8_t accountNum = 0;
    GetExistAccountNum(&accountNum);
    if (accountNum <= 0) {
        return;
    }
    if (g_fpManager.fingerNum != 0) {
        if (type == RECOGNIZE_UNLOCK && !g_fpManager.unlockFlag) {
            printf("unlocking\n");
            return;
        }
        g_fingerRecognizeType = type;
        FpRecognizeSend(FINGERPRINT_CMD_RECOGNIZE, type);
    }
}

static void SearchFpChipId(void)
{
    FpGenericSend(FINGERPRINT_CMD_GET_CHIP_ID, true);
}

void SearchFpUId(void)
{
    FpGenericSend(FINGERPRINT_CMD_GET_UID, true);
}

void SearchFpFwVersion(void)
{
    FpGenericSend(FINGERPRINT_CMD_GET_VER, false);
}

void GetFpRandomNumber(void)
{
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, NO_ENCRYPTION);
}

void FpCancelCurOperate(void)
{
    g_delayCmd = FINGERPRINT_CMD_CANCEL_EXECUTE;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
}

void SearchFpInitState(void)
{
    uint8_t reserveData = 0;
    SendPackFingerMsg(FINGERPRINT_CMD_GET_INIT_STATE, &reserveData, 0, 1, NO_ENCRYPTION);
}

void SetFpAesKey(void)
{
    uint8_t aesKey[16] = {0};
    TrngGet(aesKey, sizeof(aesKey));
    SetFpCommAesKey(aesKey);
    memcpy_s(g_communicateAesKey, sizeof(g_communicateAesKey), aesKey, 16);
}

void FingerPrintModuleRestart(void)
{
    FingerprintRestart();
    FpResponseHandleStop();
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    g_delayCmd = g_lastCmd;
    g_delayCmdTimer = osTimerNew(FpDelayTimerHandle, osTimerOnce, NULL, NULL);
    osTimerStart(g_delayCmdTimer, 150);
}

// search aes key state
static void SearchFpAesKeyState(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    ret = GetFpCommAesKey(g_communicateAesKey);
    if (ret != SUCCESS_CODE) {
        return;
    }
    memset_s(&g_communicateAesKey[16], 16, 0, 16);
    FpGenericSend(FINGERPRINT_CMD_GET_AES_KEY_STATE, NO_ENCRYPTION);
}

// set low power mode
void SetFpLowPowerMode(void)
{
    FpLowerPowerSend(FINGERPRINT_CMD_LOW_POWER, AES_KEY_ENCRYPTION);
}

// check timer
void FpTimeoutHandle(void *argument)
{
    bool timerStop = true;
    osMutexAcquire(g_fpResponseMutex, osWaitForever);
    for (uint32_t i = 1; i < NUMBER_OF_ARRAYS(g_cmdTimeoutMap); i++) {  // random number is not processed
        if (g_cmdTimeoutMap[i].cnt != FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT) {
            if (g_cmdHandleMap[i].cmd == FINGERPRINT_CMD_RECOGNIZE) {
                if (g_cmdTimeoutMap[i].cnt == 2) {
                    FpRecognize(g_fingerRecognizeType);
                    g_cmdTimeoutMap[i].cnt = 0;
                }
            } else if (g_cmdTimeoutMap[i].cnt == 10) {
                for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(g_cmdTimeoutMap); i++) {
                    g_cmdTimeoutMap[i].cnt = FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT;
                }
                switch (g_cmdHandleMap[i].cmd) {
                case FINGERPRINT_CMD_GET_REG_NUM:
                    FpGetNumberSend(FINGERPRINT_CMD_GET_REG_NUM, 0);
                    break;
                case FINGERPRINT_CMD_DELETE_SINGLE:
                    FpDeleteSend(FINGERPRINT_CMD_DELETE_SINGLE, g_fpIndex);
                    break;
                case FINGERPRINT_CMD_DELETE_ALL:
                    FpDeleteSend(FINGERPRINT_CMD_DELETE_ALL, 1);
                    break;
                default:
                    FpGenericSend(g_cmdHandleMap[i].cmd, g_cmdHandleMap[i].isEncrypt);
                    break;
                }
                // printf("cmd:%04x resend\n", g_cmdHandleMap[i].cmd);
            }
            timerStop = false;
            g_cmdTimeoutMap[i].cnt++;
        }
    }
    if (timerStop == true) {
        osTimerStop(g_fpTimeoutTimer);
    }
    osMutexRelease(g_fpResponseMutex);
}

// response handle
static void FpResponseHandle(uint16_t cmd)
{
    osMutexAcquire(g_fpResponseMutex, osWaitForever);
    for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(g_cmdTimeoutMap); i++) {
        if (g_cmdTimeoutMap[i].cmd == cmd) {
            g_cmdTimeoutMap[i].cnt = FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT;
        }
    }

    osMutexRelease(g_fpResponseMutex);
}

void FpResponseHandleStop(void)
{
    osMutexAcquire(g_fpResponseMutex, osWaitForever);
    osTimerStop(g_fpTimeoutTimer);
    for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(g_cmdTimeoutMap); i++) {
        g_cmdTimeoutMap[i].cnt = FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT;
    }

    osMutexRelease(g_fpResponseMutex);
}

// get fp data handle func
FpDataHandelFunc GetFpDataHandelFunc(uint16_t cmd, bool *isEncrypt)
{
    for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(g_cmdHandleMap); i++) {
        if (g_cmdHandleMap[i].cmd == cmd) {
            *isEncrypt = g_cmdHandleMap[i].isEncrypt;
            return g_cmdHandleMap[i].rcvFunc;
        }
    }
    *isEncrypt = true;
    return NULL;
}

// recv buff handle
void FingerprintRcvMsgHandle(char *recvBuff, uint8_t len)
{
    if (g_devLogSwitch) {
        for (int i = 0; i < len; i++) {
            printf("%02X ", recvBuff[i]);
        }
        printf("\n");
    }
    uint16_t cmd = 0;
    FpDataHandelFunc recvFunc;
    if (recvBuff[0] != FINGERPRINT_CMD_FRAME_SYNC_HEAD) {
        printf("UnPickFingerMsg error:frame head error, is 0x%2x\r\n", recvBuff[0]);
        return;
    }

    uint16_t totalDataLen = (((uint16_t)recvBuff[2]) << 8 | recvBuff[1]) + 3;
    uint32_t totalCrc = 0;
    memcpy_s(&totalCrc, sizeof(totalCrc), &recvBuff[totalDataLen - 4], sizeof(totalCrc));
    uint32_t checkCrc = crc32_update_fast((const uint8_t *)&recvBuff[0], totalDataLen - 4);
    if (totalCrc != checkCrc) {
        printf("UnPickFingerMsg error:total packet crc error, rcvCrc is 0x%X, check dataCrc is 0x%X\r\n", totalCrc, checkCrc);
        return;
    }

    FpRecvMsg_t msg = {0};
    memcpy_s(&msg, sizeof(msg), recvBuff, len);

    bool isEncrypt = false;
    recvFunc = GetFpDataHandelFunc(msg.data.cmd0 * 256 + msg.data.cmd1, &isEncrypt);
    if (recvFunc == NULL || isEncrypt == true) {
        DecryptFingerprintData((uint8_t *)&msg.data, (const uint8_t *)&recvBuff[3], len - 7);
        recvFunc = GetFpDataHandelFunc(msg.data.cmd0 * 256 + msg.data.cmd1, &isEncrypt);
    }
    cmd = msg.data.cmd0 * 256 + msg.data.cmd1;

    // response
    do {
        if (totalDataLen == FINGERPRINT_RESPONSE_MSG_LEN) {
            if (cmd == FINGERPRINT_CMD_CANCEL_EXECUTE) {
                const uint8_t cancelCmd[7] = {0xB6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                if (!memcmp(&msg.data.cmd0, cancelCmd, sizeof(cancelCmd))) {
                }
            }

            if (cmd == FINGERPRINT_CMD_GET_AES_KEY_STATE || cmd == FINGERPRINT_CMD_LOW_POWER || cmd == FINGERPRINT_CMD_SET_AES_KEY) {
                break;
            }

            FpResponseHandle(cmd);
            return;
        }
    } while (0);

    if (cmd != FINGERPRINT_CMD_GET_RANDOM_NUM) {
        // printf("%#4x len = %d recv.....\n", cmd, msg.data.dataLen);
    }
    if (recvFunc != NULL) {
        recvFunc((char *)msg.data.data, msg.data.dataLen);
    }
}

void SetFpFlashMode(bool flash)
{
    printf("set fp flash %d\n", flash);
    if (flash) {
        FpResponseHandleStop();
        Uart2DeInit();
        FingerprintRestart();
    } else {
        Uart2Init(FingerprintIsrRecvProcess);
        FingerprintRestart();
    }
}


// finger print module restart
void FingerprintRestart(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_11;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOE, &gpioInit);

    GPIO_ResetBits(GPIOE, GPIO_Pin_12);
    osDelay(10);
    GPIO_SetBits(GPIOE, GPIO_Pin_12);
    osDelay(10);

    GPIO_ResetBits(GPIOE, GPIO_Pin_11);
    osDelay(10);
    GPIO_SetBits(GPIOE, GPIO_Pin_11);
    osDelay(800);
}

// check finger press
bool FingerPress(void)
{
    return GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_14) == Bit_RESET;
}

static int SendDataToFp(uint8_t *pBuffer, int size)
{
    if (g_devLogSwitch) {
        printf("send size = %d, send data to uart: ", size);
    }
    for (int i = 0; i < size; i++) {
        while (!UART_IsTXEmpty(UART2));
        if (g_devLogSwitch) {
            printf("%02X ", pBuffer[i]);
        }
        UART_SendData(UART2, pBuffer[i]);
    }
    if (g_devLogSwitch) {
        printf("\r\n");
    }
    return size;
}

// packet msg
void SendPackFingerMsg(uint16_t cmd, uint8_t *data, uint16_t frameId, uint32_t len, Encryption_Type isEncrypt)
{
    static FpSendMsg_t sendData;
    memset_s(&sendData, sizeof(sendData), 0, sizeof(sendData));

    sendData.header    = FINGERPRINT_CMD_FRAME_SYNC_HEAD;
    sendData.packetLen =  16 + (len + 10) / 16 * 16 + 16 + 4;
    sendData.data.cmd0 = (cmd >> 8) & 0xFF;
    sendData.data.cmd1 = cmd & 0xFF;
    sendData.data.reserve = 0;
    sendData.data.frameNum = frameId;
    sendData.data.dataLen = len;
    memcpy_s(&sendData.data.data[0], MAX_MSG_DATA_LENGTH, data, sendData.data.dataLen);

    if (isEncrypt != NO_ENCRYPTION) {
        uint8_t aesKey[32] = {0};
        osDelay(10);
        UpdateHostRandom();
        memcpy_s(sendData.random, 16, GetRandomAesKey(g_hostRandomKey, g_fpRandomKey, g_communicateAesKey), 16);
        CLEAR_ARRAY(aesKey);
    }

    uint32_t protocolCRC = crc32_update_fast((const uint8_t *)&sendData.data.cmd0, sendData.packetLen - 16 - 4 - 4);
    if (g_devLogSwitch) {
        PrintArray("before encrypt", &sendData.data.cmd0, sendData.packetLen - 16 - 4 - 4);
    }
    sendData.data.signature = protocolCRC;
    if (isEncrypt != NO_ENCRYPTION) {
        EncryptFingerprintData((uint8_t *)&sendData.data.signature, (const uint8_t *)&sendData.data.signature, sendData.packetLen - 16 - 4);
    }

    uint32_t totalCrc = crc32_update_fast((const uint8_t *)&sendData.header, sendData.packetLen + 3 - 4);
    memcpy_s((uint8_t *)&sendData.data.signature + (len + 10) / 16 * 16 + 16, 4, &totalCrc, 4);

    SendDataToFp((uint8_t *)&sendData, sendData.packetLen + 3);
}

void decryptFunc(uint8_t *decryptPasscode, uint8_t *encryptPasscode, uint8_t *passwordAesKey)
{
    AES256_ctx ctx;
    AES256_init(&ctx, passwordAesKey);
    AES256_decrypt(&ctx, 2, decryptPasscode, encryptPasscode);
}

void FingerprintInit(void)
{
    g_fpResponseMutex = osMutexNew(NULL);
    g_fpEventGroup = xEventGroupCreate();

    Uart2Init(FingerprintIsrRecvProcess);
}

void __inline FingerprintIsrRecvProcess(uint8_t byte)
{
    static int32_t rcvByteCount = 0;
    static uint16_t totalLen = 0;
    static uint32_t lastTick = 0;
    static char intrRecvBuffer[RCV_MSG_MAX_LEN] = {0};
    uint32_t tick;
    if (osKernelGetState() < osKernelRunning) {
        return;
    }
    tick = osKernelGetTickCount();
    if (rcvByteCount != 0) {
        if (tick - lastTick > 200) {
            rcvByteCount = 0;
            memset_s(g_intrRecvBuffer, RCV_MSG_MAX_LEN, 0, RCV_MSG_MAX_LEN);
        }
    }
    lastTick = tick;

    lastTick = tick;
    if (rcvByteCount == 0) {   // frame head
        if (byte == 0xAA) {
            intrRecvBuffer[rcvByteCount++] = byte;
        } else {
            memset_s(intrRecvBuffer, RCV_MSG_MAX_LEN, 0, RCV_MSG_MAX_LEN);
            rcvByteCount = 0;
        }
    } else if (rcvByteCount == 1 || rcvByteCount == 2) {       //frame len
        intrRecvBuffer[rcvByteCount++] = byte;
        if (rcvByteCount == 2) {
            totalLen = (intrRecvBuffer[2] << 8) + intrRecvBuffer[1] + 3;
        }
    } else {
        intrRecvBuffer[rcvByteCount++] = byte;
        if (rcvByteCount == totalLen) {
            if (g_delayCmd == FINGERPRINT_CMD_LOW_POWER && totalLen == 0x27) {
                uint8_t passwd = 0;
                memcpy_s(g_fpRandomKey, sizeof(g_fpRandomKey), &intrRecvBuffer[15], 16);
                SendPackFingerMsg(FINGERPRINT_CMD_LOW_POWER, &passwd, 0, 1, AES_KEY_ENCRYPTION);
                g_delayCmd = 0;
                FpSendTimerStart(FINGERPRINT_CMD_LOW_POWER);
            } else {
                xEventGroupSetBitsFromISR(g_fpEventGroup, 0x1, NULL);
                memcpy_s(g_intrRecvBuffer, RCV_MSG_MAX_LEN, intrRecvBuffer, rcvByteCount);
            }
            memset_s(intrRecvBuffer, RCV_MSG_MAX_LEN, 0, RCV_MSG_MAX_LEN);
            rcvByteCount = 0;
            totalLen = 0;
        }
    }
}

void FpWipeManageInfo(void)
{
    memset_s(&g_fpManager, sizeof(g_fpManager), 0, sizeof(g_fpManager));
    SetFingerManagerInfoToSE();
    FpDeleteSend(FINGERPRINT_CMD_DELETE_ALL, 1);
}

void FingerTest(int argc, char *argv[])
{
    uint8_t index = 0;
    uint8_t value = 0;

    if (strcmp(argv[0], "reg") == 0) {
        sscanf(argv[1], "%d", &index);
        RegisterFp(index);
    } else if (strcmp(argv[0], "delete") == 0) {
        sscanf(argv[1], "%d", &index);
        DeleteFp(index);
    } else if (strcmp(argv[0], "delete_all") == 0) {
        DeleteFp(0xFF);
    } else if (strcmp(argv[0], "fp_num") == 0) {
        SearchFpNum();
    } else if (strcmp(argv[0], "recognize") == 0) {
        sscanf(argv[1], "%d", &index);
        FpRecognize((Recognize_Type)index);
    } else if (strcmp(argv[0], "low_power") == 0) {
        SetFpLowPowerMode();
    } else if (strcmp(argv[0], "cancel") == 0) {
        FpCancelCurOperate();
    } else if (strcmp(argv[0], "fp_reset") == 0) {
        FingerprintRestart();
        printf("finger restart\r\n");
#ifdef ENABLE_FP_RESET
    } else if (strcmp(argv[0], "sys_reset") == 0) {
        FpGenericSend(FINGERPRINT_CMD_SYS_RESET, true);
        printf("clear done\r\n");
#endif
    } else if (strcmp(argv[0], "set_aes_key") == 0) {
        FpSetAesKeySend(0, 0);
    } else if (strcmp(argv[0], "aes_state") == 0) {
        SearchFpAesKeyState();
    } else if (strcmp(argv[0], "chip_id") == 0) {
        SearchFpChipId();
    } else if (strcmp(argv[0], "uid") == 0) {
        SearchFpUId();
    } else if (strcmp(argv[0], "ver") == 0) {
        SearchFpFwVersion();
    } else if (strcmp(argv[0], "init_state") == 0) {
        SearchFpInitState();
    } else if (strcmp(argv[0], "dev_log_switch") == 0) {
        g_devLogSwitch = !g_devLogSwitch;
    } else if (strcmp(argv[0], "flash") == 0) {
        sscanf(argv[1], "%d", &value);
        SetFpFlashMode(!!value);
    } else if (strcmp(argv[0], "random") == 0) {
        GetFpRandomNumber();
    }
}
