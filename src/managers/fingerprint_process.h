#ifndef _FINGER_PROCESS_H
#define _FINGER_PROCESS_H

#include "stdint.h"
#include "stdbool.h"

#define MAX_NUM_FINGERS                         (3)
#define MAX_NUM_WALLETS                         (3)

#define RCV_MSG_MAX_LEN                         (512)
#define MAX_SEND_DATA_LENGTH                    (229)
#define RCV_ACK_TIMEOUT                         (500)
#define MAX_MSG_DATA_LENGTH                     (240)

#define FINGERPRINT_SUCCESS                     (0)
#define FINGERPRINT_SEND_ERROR                  (-1)
#define FINGERPRINT_RCV_ACK_TIMEOUT             (-2)
#define FINGERPRINT_RCV_ACK_ERROR               (-3)
#define FINGERPRINT_RCV_TIMEOUT                 (-4)
#define FINGERPRINT_RCV_CRC_ERROR               (-5)
#define FINGERPRINT_RCV_PTC_CRC_ERROR           (-6)
#define FINGERPRINT_RCV_CMD_ERROR               (-7)
#define FINGERPRINT_RCV_ERROR                   (-8)

#define FINGERPRINT_CMD_FRAME_SYNC_HEAD         (0XAA)

#define FINGERPRINT_CMD_REG                     (0xA000)
#define FINGERPRINT_CMD_DELETE                  (0xA4)
#define FINGERPRINT_CMD_DELETE_SINGLE           (0xA400)
#define FINGERPRINT_CMD_DELETE_ALL              (0xA401)
#define FINGERPRINT_CMD_GET_REG_NUM             (0xA500)
#define FINGERPRINT_CMD_RECOGNIZE               (0xA7FF)

#define FINGERPRINT_CMD_GET_CHIP_ID             (0xB100)
#define ENABLE_FP_RESET
#ifdef ENABLE_FP_RESET
#define FINGERPRINT_CMD_SYS_RESET               (0xB200)
#endif
#define FINGERPRINT_CMD_GET_UID                 (0xB400)
#define FINGERPRINT_CMD_GET_VER                 (0xB500)
#define FINGERPRINT_CMD_CANCEL_EXECUTE          (0xB600)

#define FINGERPRINT_CMD_PARAM_RESET             (0xC000)
#define FINGERPRINT_CMD_GET_INIT_STATE          (0xC200)

#define FINGERPRINT_CMD_SET_AES_KEY             (0xD000)
// #define FINGERPRINT_CMD_SET_RESET_AES_KEY       (0xD002)
#define FINGERPRINT_CMD_GET_AES_KEY_STATE       (0xD100)
#define FINGERPRINT_CMD_GET_RANDOM_NUM          (0xD200)
#define FINGERPRINT_CMD_SYS_TEST                (0xD300)
#define FINGERPRINT_CMD_LOW_POWER               (0xD400)

#define FINGERPRINT_RESPONSE_MSG_LEN            (23)
#define FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT    (0xFF)
#define FINGERPRINT_EN_SING_ERR_TIMES           (5)
#define FINGERPRINT_SING_ERR_TIMES              (3)
#define FINGERPRINT_SING_DISABLE_ERR_TIMES      (15)

typedef enum {
    FP_SUCCESS_CODE = 0,

    ERR_FP_NO_FINGER_PRESS = 0x11,
    ERR_FP_LIFT_FINGER_TOO_FAST = 0x14,
    ERR_FP_FINGER_LEAVE = 0x17,

    ERR_FP_NUMBER_OF_FINGER_ZERO = 0x2D,
    ERR_FP_ALGORITHMIC_PROCESSING_FAILURE = 0x30,

    ERR_FP_AREA_TOO_WET = 0x38,
    ERR_FP_RECORDING_AREA_SMALL = 0x39,
    ERR_FP_ARE_POOR_QUALITY = 0x3A,
    ERR_FP_RECORDING_AREA_OVERLAPS_TOO_MUCH = 0x3B,
    ERR_FP_TOO_MUCH_WEIGHT_ON_RECORDING_AREA = 0x3C,

    ERR_FP_TAKEN_YOUR_FINGER_OFF_IT = 0x50,
    ERR_FP_FIRMWARE_UPGRADE_FAILURE = 0x51,
    ERR_FP_FIRMWARE_OVERSIZE = 0x52,
    ERR_FP_TEMPLATE_SAVING_ERROR = 0x53,
    ERR_FP_ENROLL_ERROR = 0x54,
    ERR_FP_ENROLL_FINISH_ERROR = 0x55,
    ERR_FP_GENERAL_FAIL = 0x56,

    ERR_FP_FLASH_MESSAGE_FAILURE = 0x80,
    ERR_FP_NO_TEMPLATES_WERE_MATCHED = 0x81,
    ERR_FP_NON_EXISTENT_FINGERPRINT = 0x84,
    ERR_FP_AES_KEY_ALREADY_EXISTS = 0x8B,

    ERR_FP_REPEAT_FINGER = 0x93,
    ERR_FP_END,
} FingerError_Code;

typedef void (*FpDataSendFunc)(uint16_t cmd, uint8_t param);
typedef void (*FpDataHandelFunc)(char *indata, uint8_t len);

typedef struct {
    uint16_t cmd;
    bool isEncrypt;
    FpDataSendFunc sendFunc;
    FpDataHandelFunc rcvFunc;
} FingerPrintControl_t;

typedef struct {
    uint32_t signature;
    uint8_t cmd0;
    uint8_t cmd1;
    uint8_t reserve;
    uint16_t frameNum;
    uint16_t dataLen;
    uint8_t data[MAX_MSG_DATA_LENGTH];
#ifdef COMPILE_SIMULATOR
} FpEncryptData_t;
#else
} __attribute__((packed, aligned(1))) FpEncryptData_t;
#endif

typedef struct {
    uint8_t header;
    uint16_t packetLen;
    uint8_t random[16];
    FpEncryptData_t data;
#ifdef COMPILE_SIMULATOR
} FpSendMsg_t;
#else
} __attribute__((packed, aligned(1))) FpSendMsg_t;
#endif

typedef struct {
    uint8_t header;
    uint16_t packetLen;
    FpEncryptData_t data;
#ifdef COMPILE_SIMULATOR
} FpRecvMsg_t;
#else
} __attribute__((packed, aligned(1))) FpRecvMsg_t;
#endif

typedef struct {
    uint16_t cmd;
    uint8_t cnt;
} FingerPrintTimeout_t;

typedef enum {
    RECOGNIZE_UNLOCK = 0,
    RECOGNIZE_OPEN_SIGN,
    RECOGNIZE_SIGN,
} Recognize_Type;

typedef enum {
    NO_ENCRYPTION,
    AES_KEY_ENCRYPTION,
} Encryption_Type;

typedef struct {
    bool unlockFlag;
    bool signFlag[MAX_NUM_WALLETS];
    uint8_t fingerNum;
    uint8_t fingerId[MAX_NUM_FINGERS];
    uint8_t reserve[24];
} FingerManagerInfo_t;

void SetFingerManagerInfoToSE(void);
uint8_t GetFingerUnlockFlag(void);
void UpdateFingerUnlockFlag(uint8_t unlockFlag);
uint8_t GetFingerSignFlag(void);
void UpdateFingerSignFlag(uint8_t index, bool signFlag);
uint8_t GetRegisteredFingerNum(void);
void UpdateRegisteredFingerNum(uint8_t num);
uint8_t GetFingerRegisteredStatus(uint8_t fingerIndex);
void UpdateFingerRegisteredStatus(uint8_t fingerIndex, uint8_t data);
void FpDeleteRegisterFinger(void);

void decryptFunc(uint8_t *decryptPasscode, uint8_t *encryptPasscode, uint8_t *passwordAesKey);
void FingerSetInfoToSE(uint8_t *random, uint8_t fingerId, uint8_t accountIndex, char *password);
bool FingerPress(void);
void UpdateHostRandom(void);
void FingerprintRestart(void);
void FingerprintInit(void);
void FingerprintRcvMsgHandle(char *recvBuff, uint8_t len);
void FingerprintIsrRecvProcess(uint8_t byte);
void SendPackFingerMsg(uint16_t cmd, uint8_t *data, uint16_t frameId, uint32_t len, Encryption_Type isEncrypt);
void FingerTest(int argc, char *argv[]);
void InitFingerManagerInfo(void);
bool FpModuleIsExist(void);
bool FpModuleIsChipState(void);

void RegisterFp(uint8_t index);
void DeleteFp(uint8_t index);
void FpRecognize(Recognize_Type type);
void SearchFpFwVersion(void);
void FpCancelCurOperate(void);
void SearchFpInitState(void);
void SetFpAesKey(void);
void SetFpLowPowerMode(void);
uint8_t *GuiGetFpVersion(char *version);
void FpWipeManageInfo(void);
const char *GetFpErrorMessage(FingerError_Code errCode);
void FpResponseHandleStop(void);
void FpSaveKeyInfo(bool add);

#endif