/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_model.c
 * Description:
 * author     : stone wang
 * data       : 2023-03-23 09:41
**********************************************************************/
#include <string.h>
#include "stdlib.h"
#include "gui.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_api.h"
#include "bip39_english.h"
#include "user_memory.h"
#include "log_print.h"
#include "secret_cache.h"
#include "background_task.h"
#include "bip39.h"
#include "slip39.h"

#include "account_public_info.h"
#include "user_utils.h"

#include "device_setting.h"
#include "user_memory.h"
#include "presetting.h"
#include "ui_display_task.h"
#include "motor_manager.h"
#include "gui_animating_qrcode.h"
#include "screen_manager.h"
#include "keystore.h"
#ifndef COMPILE_SIMULATOR
#include "rust.h"
#include "user_msg.h"
#include "general_msg.h"
#include "cmsis_os.h"
#include "fingerprint_process.h"
#include "user_delay.h"
#include "user_fatfs.h"
#endif

static PasswordVerifyResult_t g_passwordVerifyResult;

static int32_t ModeGetAmount(const void *inData, uint32_t inDataLen);

#ifndef COMPILE_SIMULATOR
#define MODEL_WRITE_SE_HEAD                 do {                                \
        ret = CHECK_BATTERY_LOW_POWER();                                        \
        CHECK_ERRCODE_BREAK("save low power", ret);                             \
        ret = GetBlankAccountIndex(&newAccount);                                \
        CHECK_ERRCODE_BREAK("get blank account", ret);                          \
        ret = GetExistAccountNum(&accountCnt);                                  \
        CHECK_ERRCODE_BREAK("get exist account", ret);                          \
        printf("before accountCnt = %d\n", accountCnt);

#define MODEL_WRITE_SE_END                                                      \
        ret = VerifyPasswordAndLogin(&newAccount, SecretCacheGetNewPassword());    \
        CHECK_ERRCODE_BREAK("login error", ret);                                \
        GetExistAccountNum(&accountCnt);                                        \
        printf("after accountCnt = %d\n", accountCnt);                          \
    } while (0);                                                                \
    if (ret == SUCCESS_CODE) {                                                  \
        ClearSecretCache();                                                     \
        GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));  \
    } else {                                                                            \
        GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));     \
    }
#endif

#ifdef COMPILE_SIMULATOR
int32_t AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen)
{
    func(inData, inDataLen);
    return SUCCESS_CODE;
}

int32_t AsyncExecuteRunnable(BackgroundAsyncFuncWithRunnable_t func, const void *inData, uint32_t inDataLen, BackgroundAsyncRunnable_t runnable)
{
    return SUCCESS_CODE;
}
#endif

void GuiModeGetAmount(void)
{
    AsyncExecute(ModeGetAmount, NULL, 0);
}

// get wallet amount
static int32_t ModeGetAmount(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    static uint8_t walletAmount;
#ifndef COMPILE_SIMULATOR
    int32_t ret;

    ret = GetExistAccountNum(&walletAmount);
    if (ret != SUCCESS_CODE) {
        walletAmount = 0xFF;
    }
    GuiApiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
#else
    walletAmount = SIMULATOR_WALLET_AMOUNT;
    GuiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

