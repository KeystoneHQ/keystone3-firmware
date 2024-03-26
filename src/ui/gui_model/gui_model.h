#ifndef _GUI_MODEL_H
#define _GUI_MODEL_H

#ifndef COMPILE_SIMULATOR

#include "fingerprint_process.h"
#include "screen_manager.h"
#include "anti_tamper.h"
#include "presetting.h"
#include "drv_sdcard.h"

#include "presetting.h"
#include "anti_tamper.h"
#else
#include "simulator_model.h"
#endif
#include "drv_rtc.h"
#include "drv_battery.h"
#include "account_manager.h"

#define MAX_LOGIN_PASSWORD_ERROR_COUNT  10
#define MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX 4
#define MAX_CURRENT_PASSWORD_ERROR_COUNT_WIPE_DEVICE 14

typedef struct {
    uint8_t threShold;
    uint8_t memberCnt;
    uint8_t wordCnt;
    bool forget;
} Slip39Data_t;

typedef struct {
    uint8_t wordCnt;
    bool forget;
} Bip39Data_t;

typedef struct {
    uint8_t iconIndex;
    char name[WALLET_NAME_MAX_LEN + 1];
} WalletDesc_t;

typedef struct PasswordVerifyResult {
    void *signal;
    uint16_t errorCount;
} PasswordVerifyResult_t;

typedef void *(*ReturnVoidPointerFunc)(void);

void GuiModeGetAccount(void);

#endif /* _GUI_MODEL_H */

