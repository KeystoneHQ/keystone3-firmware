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
#define JSON_MAX_LEN (1024 * 100)
#define ACCOUNT_PUBLIC_HOME_COIN_PATH "C:/assets/coin.json"

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
uint8_t GetCurrentAccountIndex(void);
uint8_t GetBatterPercent(void);
int Slip39CheckFirstWordList(char *wordsList, uint8_t wordCnt, uint8_t *threshold);
bool GetPassphraseQuickAccess(void);
bool SdCardInsert(void);
void SdCardIntHandler(void);
bool UsbInitState(void);
void CloseUsb();
void UpdateFingerSignFlag(uint8_t index, bool signFlag);

#define LOW_BATTERY_LIMIT               20
#define CHECK_BATTERY_LOW_POWER()       ((GetBatterPercent() <= LOW_BATTERY_LIMIT) ? ERR_KEYSTORE_SAVE_LOW_POWER : SUCCESS_CODE)
#define SIMULATOR_WALLET_AMOUNT         1

extern bool g_reboot;

#endif