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

extern osTimerId_t g_fpTimeoutTimer;
static osMutexId_t g_fpResponseMutex;
EventGroupHandle_t g_fpEventGroup = NULL;

#define FINGERPRINT_REG_MAX_TIMES               (18)

static uint8_t g_errorCnt = 0;
static osTimerId_t g_delayCmdTimer = NULL;
static bool g_fpStatusExist = false;

static int SendDataToFp(uint8_t *pBuffer, int size);
static uint8_t *GetRandomAesKey(uint8_t *hostRandomKey, uint8_t *fpRandomKey, uint8_t *communicateAesKey);
static void EncryptFingerprintData(uint8_t *cipherData, const uint8_t *plainData, uint16_t len);
void FingerPrintOtaSend(void);
void FpGenericSend(uint16_t cmd, uint8_t isEncrypt);
void FpGenericRecv(char *indata, uint8_t len);
void FpUpdateUpdateRandom(char *indata, uint8_t len);
void FpGetChipId(char *indata, uint8_t len);
void FpGetVersion(char *indata, uint8_t len);
void FpRegSend(uint16_t cmd, uint8_t param);
void FpRegRecv(char *indata, uint8_t len);
void FpDeleteSend(uint16_t cmd, uint8_t index);
void FpDeleteRecv(char *indata, uint8_t len);
void FpGetNumberSend(uint16_t cmd, uint8_t fingerInfo);
void FpGetNumberRecv(char *indata, uint8_t len);
void FpRecognizeSend(uint16_t cmd, uint8_t passwd);
void FpRecognizeRecv(char *indata, uint8_t len);
void FpGetUid(char *indata, uint8_t len);
void FpCancelRecv(char *indata, uint8_t len);
void FpGetAesKeyState(char *indata, uint8_t len);
void FpGetInitStateRecv(char *indata, uint8_t len);
void DeleteFingerManager(uint8_t index);
void FpUpdateOtaCmdResponse(char *indata, uint8_t len);
void AddFingerManager(uint8_t index);
bool GuiLockScreenIsTop(void);
void FpResponseHandle(uint16_t cmd);
void FpLowerPowerRecv(char *indata, uint8_t len);
void FpLowerPowerSend(uint16_t cmd, uint8_t param);
void FpSetAesKeySend(uint16_t cmd, uint8_t fingerInfo);
void FpSetAesKeyRecv(char *indata, uint8_t len);
void FpDeleteAllRecv(char *indata, uint8_t len);
void FpResponseHandleStop(void);
void FpUpdateFirmwareSend(uint16_t cmd, uint8_t passwd);

int32_t SetFpStartOta(void);
void FpDelayMsgSend(void);

static uint8_t g_fpDeleteIndex;
static uint8_t g_fpRegIndex;
static uint16_t g_delayCmd;
static uint16_t g_lastCmd;
static bool g_isOta = false;
static bool g_devLogSwitch = false;
char g_intrRecvBuffer[RCV_MSG_MAX_LEN] = {0};
static uint8_t g_fpRandomKey[16] = {0};
static uint8_t g_hostRandomKey[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10};
static uint8_t g_randomAesKey[16] = {0};
static Recognize_Type g_fingerRecognizeType = RECOGNIZE_UNLOCK;
static uint8_t g_fpRegCnt = 0;
FingerManagerInfo_t g_fpManager = {0};
static uint8_t g_fpVersion[4] = {0};

static void FpDelayTimerHandle(void *argument)
{
    FpDelayMsgSend();
}

// todo need to store in flash
static uint8_t g_communicateAesKey[32] = {0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};

static const FingerPrintControl_t g_cmdHandleMap[] = {
    {FINGERPRINT_CMD_GET_RANDOM_NUM,    false,  FpGenericSend,      FpUpdateUpdateRandom},    //random number 优先级较高
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
    // {FINGERPRINT_CMD_UPDATE_FIRMWARE,   true,   FpUpdateFirmwareSend, FpUpdateOtaCmdResponse},
    {FINGERPRINT_CMD_GET_INIT_STATE,    true,   FpGenericSend,      FpGetInitStateRecv},
    {FINGERPRINT_CMD_SET_AES_KEY,       false,  FpSetAesKeySend,    FpSetAesKeyRecv},
    {FINGERPRINT_CMD_GET_AES_KEY_STATE, false,  FpGenericSend,      FpGetAesKeyState},
    // {FINGERPRINT_CMD_SYS_TEST,          FpCmdResponse},
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
    // {FINGERPRINT_CMD_PARAM_RESET,        FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    // {FINGERPRINT_CMD_UPDATE_FIRMWARE,       FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_GET_INIT_STATE,        FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_SET_AES_KEY,           FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_GET_AES_KEY_STATE,     FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    // {FINGERPRINT_CMD_SYS_TEST,           FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
    {FINGERPRINT_CMD_LOW_POWER,             FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT},
};

bool FpModuleIsExist(void)
{
    return g_fpStatusExist;
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
void FpRegSend(uint16_t cmd, uint8_t param)
{
    UNUSED(cmd);
    g_fpRegCnt = 0;
    g_delayCmd = FINGERPRINT_CMD_REG;
    g_fpRegIndex = param;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    FpSendTimerStart(FINGERPRINT_CMD_REG);
    // SendPackFingerMsg(FINGERPRINT_CMD_REG, &param, 0, 1, AES_KEY_ENCRYPTION);
}

// A000 RECV
void FpRegRecv(char *indata, uint8_t len)
{
    printf("%s %d\n", __func__, __LINE__);
    int i = 0;
    uint8_t result = indata[i++];
    uint8_t fingerId;
    g_fpRegCnt++;
    ClearLockScreenTime();
    if (result == FP_SUCCESS_CODE) {
        fingerId = indata[i++];
        uint8_t cnt = indata[i];
        printf("finger index = %d\n", indata[i]);
        // printf("it is been pressed %d times\n", indata[i++]);
        // printf("this time the score is %d\n", indata[i++]);
        // printf("this time the area is %d\n", indata[i++]);
        MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_SHORT_TIME);
        if (len == 38) {
            printf("save account index = %d\n", GetCurrentAccountIndex());
            AddFingerManager(fingerId);
            SetFingerManagerInfoToSE();
            FingerSetInfoToSE((uint8_t *)&indata[6], fingerId, GetCurrentAccountIndex(), SecretCacheGetPassword());
            // printf("finger aes password key:\n");
            // for (i = 6; i < len; i++) {
            //     printf("%#X ", indata[i]);
            // }
            // todo clear password
        }
        GuiApiEmitSignal(SIG_FINGER_REGISTER_STEP_SUCCESS, &cnt, sizeof(cnt));
    } else {
        printf("this time register failed\n");
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
void FpDeleteSend(uint16_t cmd, uint8_t index)
{
    g_delayCmd = cmd;
    g_fpDeleteIndex = index;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    FpSendTimerStart(cmd);
    // SendPackFingerMsg(cmd, &index, 0, 1, AES_KEY_ENCRYPTION);
}

// A400 RECV
void FpDeleteRecv(char *indata, uint8_t len)
{
    printf("%s %d\n", __func__, __LINE__);
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        printf("delete success\n");
        DeleteFingerManager(g_fpDeleteIndex);
        SetFingerManagerInfoToSE();
        if (g_fpManager.fingerNum == 0) {
            memset(&g_fpManager, 0, sizeof(g_fpManager));
        }
        GuiApiEmitSignal(SIG_FINGER_DELETE_SUCCESS, NULL, 0);
    }
}

void FpDeleteAllRecv(char *indata, uint8_t len)
{
    printf("%s %d\n", __func__, __LINE__);
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        printf("delete success\n");
        memset(&g_fpManager, 0, sizeof(g_fpManager));
    }
}

// A500 SEND
// fingerInfo 0 only get num 1 get info
void FpGetNumberSend(uint16_t cmd, uint8_t fingerInfo)
{
    g_delayCmd = FINGERPRINT_CMD_GET_REG_NUM;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    // uint16_t cmd = FINGERPRINT_CMD_GET_REG_NUM;
    // if (fingerInfo == 1) {
    //     cmd = FINGERPRINT_CMD_GET_REG_NUM + 1;
    // }
    // SendPackFingerMsg(cmd, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
    FpSendTimerStart(cmd);
}

// A500 RECV
void FpGetNumberRecv(char *indata, uint8_t len)
{
    printf("%s %d\n", __func__, __LINE__);
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        FpResponseHandle(FINGERPRINT_CMD_GET_REG_NUM);
        printf("fingerprints currently exist %d\n", indata[1]);
        GuiApiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
        if (g_fpManager.fingerNum == 0) {
            memset(&g_fpManager, 0, sizeof(g_fpManager));
        }
        if (indata[1] != g_fpManager.fingerNum) {
            memset(&g_fpManager, 0, sizeof(g_fpManager));
            SetFingerManagerInfoToSE();
            FpDeleteSend(FINGERPRINT_CMD_DELETE_ALL, 1);
        }
        // if (indata[1] != g_fpManager.fingerNum) {
        //     printf("fingerprints num not match fp: %d se: %d\n", indata[1], g_fpManager.fingerNum);
        //     g_fpManager.fingerNum = indata[1];
        //     SetFingerManagerInfoToSE();
        // }
    }
}

// A7FF SEND
// passwd 0 not need passwd 1 need passwd
void FpRecognizeSend(uint16_t cmd, uint8_t passwd)
{
    g_delayCmd = FINGERPRINT_CMD_RECOGNIZE;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    FpSendTimerStart(FINGERPRINT_CMD_RECOGNIZE);
    // SendPackFingerMsg(FINGERPRINT_CMD_RECOGNIZE, &passwd, 0, 1, AES_KEY_ENCRYPTION);
}

// A7FF RECV
void FpRecognizeRecv(char *indata, uint8_t len)
{
    int i = 0;
    uint8_t result = indata[i++];
    ClearLockScreenTime();
    if (result == FP_SUCCESS_CODE) {
        MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_SHORT_TIME);
        printf("recognize score is %d\n", indata[i++]);
        if (len == 38) {
            if (g_fingerRecognizeType == RECOGNIZE_OPEN_SIGN) {
                printf("open sign\n");
                if (GetCurrentAccountIndex() != 0xFF) {
                    FingerSetInfoToSE((uint8_t *)&indata[6], 0, GetCurrentAccountIndex(), SecretCacheGetPassword());
                } else {
                    return;
                }
            } else {
                uint8_t encryptedPassword[32] = {0};
                char decryptPasscode[128] = {0};
                GetFpEncryptedPassword(GetCurrentAccountIndex(), encryptedPassword);
                decryptFunc((uint8_t *)decryptPasscode, encryptedPassword, (uint8_t *)&indata[6]);
                SecretCacheSetPassword(decryptPasscode);
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
void FpSetAesKeySend(uint16_t cmd, uint8_t fingerInfo)
{
    memset(g_communicateAesKey, 0, sizeof(g_communicateAesKey));
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

void FpGetVersion(char *indata, uint8_t len)
{
    static bool firstIn = true;
    if (firstIn) {
        SearchFpAesKeyState();
        firstIn = false;
        g_fpStatusExist = true;
    }
    printf("version:%d.%d.%d.%d\n", indata[0], indata[1], indata[2], indata[3]);
    memcpy(g_fpVersion, indata, sizeof(g_fpVersion));
}

void FpGetAesKeyState(char *indata, uint8_t len)
{
    printf("%s %d\n", __func__, __LINE__);
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

void FpDelayMsgSend(void)
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
        SendPackFingerMsg(FINGERPRINT_CMD_DELETE_SINGLE, &g_fpDeleteIndex, 0, 1, AES_KEY_ENCRYPTION);
        FpSendTimerStart(FINGERPRINT_CMD_DELETE_SINGLE);
        g_delayCmd = 0;
    } else if (g_delayCmd == FINGERPRINT_CMD_REG) {
        SendPackFingerMsg(FINGERPRINT_CMD_REG, &g_fpRegIndex, 0, 1, AES_KEY_ENCRYPTION);
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
    } else if (g_delayCmd == FINGERPRINT_CMD_UPDATE_FIRMWARE) {
        g_delayCmd = 0;
    }
}

void FpUpdateUpdateRandom(char *indata, uint8_t len)
{
    uint8_t result = indata[0];
    if (result == FP_SUCCESS_CODE) {
        // for (int i = 1; i < len; i++) {
        //     printf("%02X ", indata[i]);
        // }
        // printf("\n");
        memcpy(g_fpRandomKey, &indata[1], 16);
        FpDelayMsgSend();
    } else {
        printf("update random failed\n");
        GetFpErrorMessage(result);
    }
}

void FpGetChipId(char *indata, uint8_t len)
{
    printf("chip id is :");
    for (int i = 0; i < len; i++) {
        printf("%X", indata[i]);
    }
    printf("\n");
}

void FpGetUid(char *indata, uint8_t len)
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

void FpCancelRecv(char *indata, uint8_t len)
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

void FpGetInitStateRecv(char *indata, uint8_t len)
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
        printf("%s %d\n", __func__, __LINE__);
        LowerPowerTimerStart();
    }
}

void PrintMsg(FpEncryptData_t *msg)
{
    printf("msg: cmd = 0x%02X%02X, frameIndex = %d, dataLen = %d\n", msg->cmd0, msg->cmd1, msg->frameNum, msg->dataLen);
    // if (msg->dataLen != 0) {
    //     printf("data: ");
    //     for (int j = 0; j < msg->dataLen; j++) {
    //         printf("%02X ", msg->data[j]);
    //     }
    //     printf("\r\n");
    // }
}

// 获取指纹信息
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

void AddFingerManager(uint8_t index)
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

void DeleteFingerManager(uint8_t index)
{
    // g_fpManager.signFlag[index] = false;
    g_fpManager.fingerNum--;
    g_fpManager.fingerId[index] = 0;
}

// 设置指纹SE
void SetFingerManagerInfoToSE(void)
{
    int32_t ret = SetFpStateInfo((uint8_t *)&g_fpManager);
    PRINT_ERRCODE(ret);
    assert(ret == 0);
}

// 获取指纹的解锁flag
uint8_t GetFingerUnlockFlag(void)
{
    return g_fpManager.unlockFlag;
}

// 更新指纹的解锁flag
void UpdateFingerUnlockFlag(uint8_t unlockFlag)
{
    g_fpManager.unlockFlag = unlockFlag;
    SetFingerManagerInfoToSE();
}

// 获取指纹的签名flag
uint8_t GetFingerSignFlag(void)
{
    return g_fpManager.signFlag[GetCurrentAccountIndex()];
}

// 更新指纹签名的flag
void UpdateFingerSignFlag(uint8_t index, bool signFlag)
{
    g_fpManager.signFlag[index] = signFlag;
    SetFingerManagerInfoToSE();
}

// 获取指纹的个数
uint8_t GetRegisteredFingerNum(void)
{
    return g_fpManager.fingerNum;
}

// 更新指纹个数
void UpdateRegisteredFingerNum(uint8_t num)
{
    g_fpManager.fingerNum = num;
}

// 获取指纹注册状态
uint8_t GetFingerRegisteredStatus(uint8_t fingerIndex)
{
    return g_fpManager.fingerId[fingerIndex];
}

// 更新指纹注册状态
void UpdateFingerRegisteredStatus(uint8_t fingerIndex, uint8_t data)
{
    g_fpManager.fingerId[fingerIndex] = data;
}

// 设置指纹的注册状态到SE
void FingerSetInfoToSE(uint8_t *random, uint8_t fingerId, uint8_t accountIndex, char *password)
{
    uint8_t cipherData[32] = {0};
    uint8_t plainData[128] = {0};

    strcpy((char *)plainData, password);
    AES256_ctx ctx;
    AES256_init(&ctx, random);
    AES256_encrypt(&ctx, 2, cipherData, plainData);
    int32_t ret = SetFpEncryptedPassword(accountIndex, cipherData);
    PRINT_ERRCODE(ret);
    assert(ret == 0);
}

// 更新的随机数
void UpdateHostRandom(void)
{
    TrngGet(g_hostRandomKey, 16);
}

// 获取随机的AESkey
static uint8_t *GetRandomAesKey(uint8_t *hostRandomKey, uint8_t *fpRandomKey, uint8_t *communicateAesKey)
{
    if (communicateAesKey == NULL) {
        memset(g_randomAesKey, 0, 16);
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

// 对数据进行加密
static void EncryptFingerprintData(uint8_t *cipherData, const uint8_t *plainData, uint16_t len)
{
    AES128_ctx ctx;
    AES128_init(&ctx, g_hostRandomKey);

    AES128_encrypt(&ctx, len / 16, cipherData, plainData);
}

// 对数据进行解密
static void DecryptFingerprintData(uint8_t *decipheredData, const uint8_t *cipherData, uint16_t len)
{
    AES128_ctx ctx;
    AES128_init(&ctx, g_hostRandomKey);
    AES128_decrypt(&ctx, len / 16, decipheredData, cipherData);
}

void FpUpdateOtaCmdResponse(char *indata, uint8_t len)
{
    printf("%s...\n", __func__);
    // FingerPrintOtaSend();
}

int32_t RegisterFp(uint8_t index)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s reg finger id is %d\n", __func__, index);
    FpRegSend(FINGERPRINT_CMD_REG, index);
    return ret;
}

int32_t DeleteFp(uint8_t index)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s delete finger id is %d\n", __func__, index);
    if (index == 0xff) {
        FpDeleteSend(FINGERPRINT_CMD_DELETE_ALL, 1);
    } else {
        FpDeleteSend(FINGERPRINT_CMD_DELETE_SINGLE, index);
    }
    return ret;
}

int32_t SearchFpNum(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s\n", __func__);
    FpGetNumberSend(FINGERPRINT_CMD_GET_REG_NUM, 0);
    return ret;
}

int32_t FpRecognize(Recognize_Type type)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    uint8_t accountNum = 0;
    GetExistAccountNum(&accountNum);
    if (accountNum <= 0) {
        return ret;
    }
    if (g_fpManager.fingerNum != 0) {
        if (type == RECOGNIZE_UNLOCK && !g_fpManager.unlockFlag) {
            printf("unlocking\n");
            return ret;
        }
        g_fingerRecognizeType = type;
        FpRecognizeSend(FINGERPRINT_CMD_RECOGNIZE, type);
    }
    return ret;
}

int32_t SearchFpChipId(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s\n", __func__);
    FpGenericSend(FINGERPRINT_CMD_GET_CHIP_ID, true);
    return ret;
}

int32_t SearchFpSensorUID(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s\n", __func__);
    FpGenericSend(FINGERPRINT_CMD_GET_CHIP_ID, true);
    return ret;
}

int32_t SearchFpFwVersion(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s %d\n", __func__, __LINE__);
    FpGenericSend(FINGERPRINT_CMD_GET_VER, false);
    return ret;
}

int32_t GetFpRandomNumber(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s %d\n", __func__, __LINE__);
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, NO_ENCRYPTION);
    return ret;
}

int32_t FpCancelCurOperate(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s\n", __func__);
    g_delayCmd = FINGERPRINT_CMD_CANCEL_EXECUTE;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    // FpGenericSend(FINGERPRINT_CMD_CANCEL_EXECUTE, true);
    // SendPackFingerMsg(FINGERPRINT_CMD_CANCEL_EXECUTE, &reserveData, 0, 1, AES_KEY_ENCRYPTION);
    return ret;
}

int32_t SearchFpInitState(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    uint8_t reserveData = 0;
    SendPackFingerMsg(FINGERPRINT_CMD_GET_INIT_STATE, &reserveData, 0, 1, NO_ENCRYPTION);
    return ret;
}

void FpUpdateFirmwareSend(uint16_t cmd, uint8_t passwd)
{
    g_delayCmd = FINGERPRINT_CMD_UPDATE_FIRMWARE;
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
}

int32_t SetFpAesKey(void)
{
    uint8_t aesKey[16] = {0};
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s\n", __func__);
    TrngGet(aesKey, sizeof(aesKey));
    SetFpCommAesKey(aesKey);
    memcpy(g_communicateAesKey, aesKey, 16);
    return ret;
}

void FingerPrintModuleRestart(void)
{
    g_errorCnt = 0;
    printf("g_errorCnt = %d\n", g_errorCnt);
    printf("fingerprint restart\n");
    FingerprintRestart();
    FpResponseHandleStop();
    FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    g_delayCmd = g_lastCmd;
    g_delayCmdTimer = osTimerNew(FpDelayTimerHandle, osTimerOnce, NULL, NULL);
    osTimerStart(g_delayCmdTimer, 150);
}

// search aes key state
int32_t SearchFpAesKeyState(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s\n", __func__);
    GetFpCommAesKey(g_communicateAesKey);
    memset(&g_communicateAesKey[16], 0, 16);
    FpGenericSend(FINGERPRINT_CMD_GET_AES_KEY_STATE, NO_ENCRYPTION);
    return ret;
}

// finger module test
int32_t FpSysRegAndRecognizeTest(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s\n", __func__);
    FpGenericSend(FINGERPRINT_CMD_SYS_TEST, NO_ENCRYPTION);
    return ret;
}

// set low power mode
int32_t SetFpLowPowerMode(void)
{
    int32_t ret = FINGERPRINT_SUCCESS;
    printf("%s...................\n", __func__);
    FpLowerPowerSend(FINGERPRINT_CMD_LOW_POWER, AES_KEY_ENCRYPTION);
    return ret;
}

// check timer
void FpTimeoutHandle(void *argument)
{
    bool timerStop = true;
    osMutexAcquire(g_fpResponseMutex, osWaitForever);
    for (uint32_t i = 1; i < NUMBER_OF_ARRAYS(g_cmdTimeoutMap); i++) {  // random num先不处理
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
                    FpDeleteSend(FINGERPRINT_CMD_DELETE_SINGLE, g_fpDeleteIndex);
                    break;
                case FINGERPRINT_CMD_DELETE_ALL:
                    FpDeleteSend(FINGERPRINT_CMD_DELETE_ALL, 1);
                    break;
                default:
                    FpGenericSend(g_cmdHandleMap[i].cmd, g_cmdHandleMap[i].isEncrypt);
                    break;
                }
                g_errorCnt++;
                // printf("cmd:%04x resend\n", g_cmdHandleMap[i].cmd);
            }
            timerStop = false;
            g_cmdTimeoutMap[i].cnt++;
        }
    }
    if (timerStop == true) {
        g_errorCnt = 0;
        osTimerStop(g_fpTimeoutTimer);
    }
    osMutexRelease(g_fpResponseMutex);

    // if (g_errorCnt == 30) {
    //     g_errorCnt = 0;
    //     printf("g_errorCnt = %d\n", g_errorCnt);
    //     printf("fingerprint restart\n");
    //     FingerprintRestart();
    //     FpResponseHandleStop();
    //     FpGenericSend(FINGERPRINT_CMD_GET_RANDOM_NUM, false);
    //     g_delayCmd = g_lastCmd;
    //     g_delayCmdTimer = osTimerNew(FpDelayTimerHandle, osTimerOnce, NULL, NULL);
    //     osTimerStart(g_delayCmdTimer, 150);
    // }
}

// response handle
void FpResponseHandle(uint16_t cmd)
{
    // printf("%s cmd = %04X.....\n", __func__, cmd);
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
        // return FINGERPRINT_RCV_ERROR;
        return;
    }

    uint16_t totalDataLen = (((uint16_t)recvBuff[2]) << 8 | recvBuff[1]) + 3;
    uint32_t totalCrc = 0;
    memcpy(&totalCrc, &recvBuff[totalDataLen - 4], sizeof(totalCrc));
    uint32_t checkCrc = crc32_update_fast((const uint8_t *)&recvBuff[0], totalDataLen - 4);
    if (totalCrc != checkCrc) {
        printf("UnPickFingerMsg error:total packet crc error, rcvCrc is 0x%X, check dataCrc is 0x%X\r\n", totalCrc, checkCrc);
        // return FINGERPRINT_RCV_CRC_ERROR;
        return;
    }

    FpRecvMsg_t msg = {0};
    memcpy(&msg, recvBuff, len);

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
                    printf("cancel success.....................\n");
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
        printf("%#4x len = %d recv.....\n", cmd, msg.data.dataLen);
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


// 重启指纹模块
void FingerprintRestart(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_11;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOE, &gpioInit);
    GPIO_ResetBits(GPIOE, GPIO_Pin_11);
    osDelay(10);
    GPIO_SetBits(GPIOE, GPIO_Pin_11);
    osDelay(800);
}

// 检测指纹按下
bool FingerPress(void)
{
    return GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_14) == Bit_RESET;
}

// 发送消息到finger print
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

// 拼接消息
void SendPackFingerMsg(uint16_t cmd, uint8_t *data, uint16_t frameId, uint32_t len, Encryption_Type isEncrypt)
{
    static FpSendMsg_t sendData;
    memset(&sendData, 0, sizeof(sendData));

    // send data packet
    sendData.header    = FINGERPRINT_CMD_FRAME_SYNC_HEAD;
    sendData.packetLen =  16 + (len + 10) / 16 * 16 + 16 + 4;
    sendData.data.cmd0 = (cmd >> 8) & 0xFF;
    sendData.data.cmd1 = cmd & 0xFF;
    sendData.data.reserve = 0;
    sendData.data.frameNum = frameId;
    sendData.data.dataLen = len;
    memcpy(&sendData.data.data[0], data, sendData.data.dataLen);

    // 填写random数据
    if (isEncrypt != NO_ENCRYPTION) {
        uint8_t aesKey[32] = {0};
        // GetCommunicateAesKey(isEncrypt, aesKey);
        osDelay(10);
        if (g_isOta == false) {
            UpdateHostRandom();
        }
        memcpy(sendData.random, GetRandomAesKey(g_hostRandomKey, g_fpRandomKey, g_communicateAesKey), 16);
        CLEAR_ARRAY(aesKey);

        // if (isEncrypt == RESET_AES_KEY_ENCRYPTION) {
        //     sendData.packetLen |= 0x80;
        // }
    }

    // if (cmd != FINGERPRINT_CMD_GET_RANDOM_NUM) {
    //     printf("%#02x%02x send.......:\n", sendData.data.cmd0, sendData.data.cmd1);
    // }
    // PrintMsg(&sendData.data);
    // 填写signature数据
    uint32_t protocolCRC = crc32_update_fast((const uint8_t *)&sendData.data.cmd0, sendData.packetLen - 16 - 4 - 4);
    if (g_devLogSwitch) {
        PrintArray("before encrypt", &sendData.data.cmd0, sendData.packetLen - 16 - 4 - 4);
    }
    sendData.data.signature = protocolCRC;
    if (isEncrypt != NO_ENCRYPTION) {
        EncryptFingerprintData((uint8_t *)&sendData.data.signature, (const uint8_t *)&sendData.data.signature, sendData.packetLen - 16 - 4);
    }

    // 总的crc校验
    uint32_t totalCrc = crc32_update_fast((const uint8_t *)&sendData.header, sendData.packetLen + 3 - 4);
    memcpy((uint8_t *)&sendData.data.signature + (len + 10) / 16 * 16 + 16, &totalCrc, 4);

    SendDataToFp((uint8_t *)&sendData, sendData.packetLen + 3);
    // if (isEncrypt != NO_ENCRYPTION) {
    //     osDelay(100);
    // }
}

void decryptFunc(uint8_t *decryptPasscode, uint8_t *encryptPasscode, uint8_t *passwordAesKey)
{
    AES256_ctx ctx;
    AES256_init(&ctx, passwordAesKey);
    AES256_decrypt(&ctx, 2, decryptPasscode, encryptPasscode);
}

// 初始化指纹串口
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
            memset(g_intrRecvBuffer, 0, RCV_MSG_MAX_LEN);
        }
    }
    lastTick = tick;

    lastTick = tick;
    if (rcvByteCount == 0) {   // frame head
        if (byte == 0xAA) {
            intrRecvBuffer[rcvByteCount++] = byte;
        } else {
            memset(intrRecvBuffer, 0, RCV_MSG_MAX_LEN);
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
                memcpy(g_fpRandomKey, &intrRecvBuffer[15], 16);
                SendPackFingerMsg(FINGERPRINT_CMD_LOW_POWER, &passwd, 0, 1, AES_KEY_ENCRYPTION);
                g_delayCmd = 0;
                FpSendTimerStart(FINGERPRINT_CMD_LOW_POWER);
            } else {
                xEventGroupSetBitsFromISR(g_fpEventGroup, 0x1, NULL);
                memcpy(g_intrRecvBuffer, intrRecvBuffer, rcvByteCount);
                // for (int i = 0; i < totalLen + 3; i++) {
                //     printf("%02x ", intrRecvBuffer[i]);
                // }
                // printf("\n");
            }
            memset(intrRecvBuffer, 0, RCV_MSG_MAX_LEN);
            rcvByteCount = 0;
            totalLen = 0;
        }
    }
}

void FpWipeManageInfo(void)
{
    memset(&g_fpManager, 0, sizeof(g_fpManager));
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
    } else if (strcmp(argv[0], "fp_test") == 0) {
        FpSysRegAndRecognizeTest();
    } else if (strcmp(argv[0], "fp_reset") == 0) {
        FingerprintRestart();
        printf("finger restart\r\n");
#ifdef ENABLE_FP_RESET
    } else if (strcmp(argv[0], "sys_reset") == 0) {
        FpGenericSend(FINGERPRINT_CMD_SYS_RESET, true);
        printf("clear done\r\n");
#endif
    } else if (strcmp(argv[0], "set_aes_key") == 0) {
        // SetFpAesKey();
        FpSetAesKeySend(0, 0);
    } else if (strcmp(argv[0], "aes_state") == 0) {
        SearchFpAesKeyState();
    } else if (strcmp(argv[0], "chip_id") == 0) {
        SearchFpChipId();
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
