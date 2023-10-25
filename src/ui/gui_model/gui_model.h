/*
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * @FilePath: \project-pillar-firmware\ui\gui_model\gui_model.h
 * @Description:
 * @Author: stone wang
 * @LastEditTime: 2023-04-12 14:51:30
 */
/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_model.h
 * Description:
 * author     : stone wang
 * data       : 2023-03-23 09:41
**********************************************************************/

#ifndef _GUI_MODEL_H
#define _GUI_MODEL_H

#ifndef COMPILE_SIMULATOR
#include "drv_battery.h"
#include "user_sqlite3.h"
#include "fingerprint_process.h"
#include "screen_manager.h"
#include "anti_tamper.h"
#include "presetting.h"
#include "drv_rtc.h"
#include "drv_sdcard.h"
#include "log.h"
#include "presetting.h
#include "anti_tamper.h"
#else
#include "simulator_model.h"
#endif
#include "gui_animating_qrcode.h"

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
    char name[64];
} WalletDesc_t;

typedef struct PasswordVerifyResult {
    void *signal;
    uint16_t errorCount;
} PasswordVerifyResult_t;

void GuiModelWriteSe(void);
void GuiModelSlip39CalWriteSe(Slip39Data_t slip39);
void GuiModelBip39CalWriteSe(Bip39Data_t bip39);
void GuiModelSettingSaveWalletDesc(WalletDesc_t *wallet);
void GuiModelSettingWritePassphrase(void);
void GuiModelSettingDelWalletDesc(void);
void GuiModelLockedDeviceDelAllWalletDesc(void);
void GuiModelChangeAmountPassWord(uint8_t accountIndex);
void GuiModelVerifyAmountPassWord(uint16_t *param);
void GuiModelBip39UpdateMnemonic(uint8_t wordCnt);
void GuiModelSlip39UpdateMnemonic(Slip39Data_t slip39);
void GuiModelBip39RecoveryCheck(uint8_t wordsCnt);
void GuiModeGetWalletDesc(void);
void GuiModeGetAmount(void);
void GuiModeControlQrDecode(bool en);
void GuiModelSlip39WriteSe(void);
void GuiModelBip39ForgetPassword(uint8_t wordsCnt);
void GuiModelSlip39ForgetPassword(Slip39Data_t slip39);
void GuiModelWriteLastLockDeviceTime(uint32_t time);
void GuiModelCalculateWebAuthCode(void *webAuthData);
void GuiModelCopySdCardOta(void);
void GuiModelURGenerateQRCode(GenerateUR func);
void GuiModelURUpdate(void);
void GuiModelURClear(void);


#endif /* _GUI_MODEL_H */

