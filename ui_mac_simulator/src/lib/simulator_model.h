#ifndef _SIMULATOR_MODEL_H
#define _SIMULATOR_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "err_code.h"

extern bool g_fingerUnlockDeviceFlag;
extern bool g_fingerSingTransitionsFlag;
extern bool fingerRegisterState[3];

// typedef enum {
//     RECOGNIZE_UNLOCK = 0,
//     RECOGNIZE_SIGN,
// } Recognize_Type;

// typedef struct {
//     uint8_t unlockFlag;
//     uint8_t reserve0[3];
//     uint8_t signFlag[3];
//     uint8_t fingerNum;
//     uint8_t fingerId[3];
//     uint8_t reserve[21];
// } FingerManagerInfo_t;

size_t xPortGetFreeHeapSize(void);
int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex);
uint8_t GetBatterPercent(void);
int Slip39CheckFirstWordList(char *wordsList, uint8_t wordCnt, uint8_t *threshold);
void sha256(void *sha, const void *p, size_t size);
bool GetPassphraseQuickAccess(void);
int strcasecmp(char const *a, char const *b);
uint32_t GetCurrentStampTime(void);
void LogSetLogName(char *name);
void SetLockTimeState(bool enable);
void UsbInit(void);
uint32_t FatfsGetSize(const char *path);
bool SdCardInsert(void);
uint32_t GetBatteryMilliVolt();
void SetPageLockScreen(bool enable);
void SetLockScreen(bool enable);
bool IsPreviousLockScreenEnable(void);
void SetPageLockScreen(bool enable);
void FpDeleteRegisterFinger(void);
void FpSaveKeyInfo(bool add);
void UpdateFingerSignFlag(uint8_t index, bool signFlag);
bool GetLvglHandlerStatus(void);
int32_t InitSdCardAfterWakeup(const void *inData, uint32_t inDataLen);

#define LOW_BATTERY_LIMIT               20
#define CHECK_BATTERY_LOW_POWER()       ((GetBatterPercent() <= LOW_BATTERY_LIMIT) ? ERR_KEYSTORE_SAVE_LOW_POWER : SUCCESS_CODE)
#define SIMULATOR_WALLET_AMOUNT         1

extern bool g_reboot;

enum {
    ERROR_LOG_NOT_ENOUGH_SPACE          = 1,
    ERROR_LOG_HAVE_NO_SD_CARD,
    ERROR_LOG_EXPORT_ERROR,
};


#endif
