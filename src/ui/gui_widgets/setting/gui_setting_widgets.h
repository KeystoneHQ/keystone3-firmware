/*
 * @Description:
 * @Vesion:
 * @Author:
 * @Date: 2023-03-22 09:36:30
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-04-03 18:41:05
 * @Path:
 */
/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_setting_widgets.h
 * Description:
 * author     : stone wang
 * data       : 2023-03-20 17:19
**********************************************************************/

#ifndef _GUI_SETTING_WIDGETS_H
#define _GUI_SETTING_WIDGETS_H

#define DEVICE_SETTING_RIGHT_LABEL_MAX_LEN                      16
#define DEVICE_SETTING_MID_LABEL_MAX_LEN                        32
#define DEVICE_SETTING_LEVEL_MAX                                8

typedef enum {
    DEVICE_SETTING = 0,

    DEVICE_SETTING_WALLET_SETTING,

    DEVICE_SETTING_CHANGE_WALLET_DESC,
    DEVICE_SETTING_CHANGE_WALLET_PASS,

    // FINGERPRINT SETTINGS
    DEVICE_SETTING_FINGERPRINT_PASSCODE,
    DEVICE_SETTING_FINGER_SETTING,
    DEVICE_SETTING_FINGER_MANAGER,

    DEVICE_SETTING_FINGER_ADD_ENTER,
    DEVICE_SETTING_FINGER_ADD_SUCCESS,
    DEVICE_SETTING_FINGER_ADD_OUT_LIMIT,
    DEVICE_SETTING_FINGER_DELETE,
    DEVICE_SETTING_FINGER_SET_PATTERN,
    // RESET PASSCODE
    DEVICE_SETTING_RESET_PASSCODE,
    DEVICE_SETTING_RESET_PASSCODE_VERIFY,
    DEVICE_SETTING_RESET_PASSCODE_WARNING,
    DEVICE_SETTING_RESET_PASSCODE_SETPIN,
    DEVICE_SETTING_RESET_PASSCODE_REPEATPIN,
    DEVICE_SETTING_RESET_PASSCODE_SETPASS,
    DEVICE_SETTING_RESET_PASSCODE_REPEATPASS,
    DEVICE_SETTING_RESET_PASSCODE_CONFIRM,

    // PASSPHRASE
    DEVICE_SETTING_PASSPHRASE,
    DEVICE_SETTING_PASSPHRASE_VERIFY,
    DEVICE_SETTING_PASSPHRASE_ENTER,
    DEVICE_SETTING_PASSPHRASE_SET_END,

    // RECOVERY PHRASE CHECK
    DEVICE_SETTING_RECOVERY_PHRASE_VERIFY,
    DEVICE_SETTING_RECOVERY_METHOD_CHECK,
    DEVICE_SETTING_RECOVERY_SINGLE_PHRASE,
    DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_12WORDS,
    DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_18WORDS,
    DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_24WORDS,
    DEVICE_SETTING_RECOVERY_SHARE_PHRASE,
    DEVICE_SETTING_RECOVERY_SHARE_PHRASE_20WORDS,
    DEVICE_SETTING_RECOVERY_SHARE_PHRASE_33WORDS,

    // ADD WALLET
    DEVICE_SETTING_ADD_WALLET,
    DEVICE_SETTING_ADD_WALLET_NOTICE,
    DEVICE_SETTING_ADD_WALLET_CREATE_OR_IMPORT,
    DEVICE_SETTING_ADD_WALLET_SETPIN,
    DEVICE_SETTING_ADD_WALLET_REPEATPIN,
    DEVICE_SETTING_ADD_WALLET_SETPASS,
    DEVICE_SETTING_ADD_WALLET_REPEATPASS,
    DEVICE_SETTING_ADD_WALLET_NAME_WALLET,
    DEVICE_SETTING_ADD_WALLET_LIMIT,

    // DEL WALLET
    DEVICE_SETTING_DEL_WALLET,
    DEVICE_SETTING_DEL_WALLET_VERIFY,
    DEVICE_SETTING_DEL_WALLET_RECOVERY_METHOD_CHECK,

    DEVICE_SETTING_SYSTEM_SETTING,
    DEVICE_SETTING_CONNECT,
    DEVICE_SETTING_ABOUT,

    DEVICE_SETTING_WEB_AUTH,

    DEVICE_SETTING_BUTT,
} DEVICE_SETTING_ENUM;

void GuiSettingInit(void);
void GuiSettingRefresh(void);
int8_t GuiDevSettingPrevTile(uint8_t tileIndex);
int8_t GuiDevSettingNextTile(uint8_t tileIndex);

void WalletSettingHandler(lv_event_t *e);
void GuiSettingCloseToTargetTileView(uint8_t targetIndex);
void GuiShowKeyboardHandler(lv_event_t *e);
void GuiSettingAnimSetLabel(const char *text);

void GuiDevSettingPassCode(bool result, uint8_t tileIndex);
void GuiSettingSetPinPass(const char* buf);
void GuiSettingRepeatPinPass(const char* buf);
void GuiResettingPassWordSuccess(void);
void GuiAddWalletPasswordRepeat(bool result);
void GuiChangeWalletDesc(bool result);
void GuiDelWallet(bool result);
void GuiWritePassphrase(bool result);
void GuiChangePassWord(bool result);
void GuiAddWalletAmountLimit(void);
void GuiWalletRecoveryWriteSe(bool result);
void GuiDelWalletSetup(void);
void GuiAddWalletCreateOrImport(uint8_t walletAmount);
void GuiAddWalletGetWalletAmount(uint8_t walletAmount);
void GuiSettingDeInit(void);
void OpenSinglePhraseHandler(lv_event_t *e);
void OpenSharePhraseHandler(lv_event_t *e);
void GuiSettingFingerRegisterSuccess(void *param);
void GuiSettingFingerRegisterFail(void *param);
void GuiSettingDealFingerRecognize(void *param);
//forget passcode share logic;
void GuiSettingCloseSelectAmountHintBox();
void GuiSettingRecoveryCheck(void);
void CloseToSubtopViewHandler(lv_event_t *e);

void GuiVerifyCurrentPasswordErrorCount(void *param);

// wallet name and icon setting
void *GuiWalletNameWallet(lv_obj_t *parent, uint8_t tile);
void GuiWalletNameWalletDestruct(void);
void GuiWalletSettingSetIconLabel(const lv_img_dsc_t *src, const char *name);

// fp and passcode setting
void GuiWalletSetFingerPassCodeWidget(lv_obj_t *parent);
void GuiWalletFingerAddWidget(lv_obj_t *parent);
uint8_t GuiGetFingerSettingIndex(void);
void GuiFingerAddDestruct(void *obj, void *param);
void GuiWalletFingerAddFpWidget(lv_obj_t *parent, bool success);
void GuiWalletFingerDeleteWidget(lv_obj_t *parent);
void GuiWalletFingerManagerWidget(lv_obj_t *parent);
void GuiFingerMangerStructureCb(void *obj, void *param);
void CancelVerifyFingerHandler(lv_event_t *e);
void CancelCurFingerHandler(lv_event_t *e);
void FingerSignHandler(lv_event_t *e);
void GuiFingerManagerDestruct(void *obj, void *param);
void GuiFpVerifyDestruct(void);

// set passphrase
void GuiWalletPassphrase(lv_obj_t *parent);
void GuiWalletPassphraseEnter(lv_obj_t *parent);

// seed check
void GuiWalletRecoveryMethodCheck(lv_obj_t *parent);
void ResetSeedCheckImportHandler(lv_event_t *e);
void GuiWalletSeedCheckClearKb(void);
void GuiWalletSeedCheckClearObject(void);
void *GuiWalletRecoverySinglePhrase(lv_obj_t *parent, uint8_t wordAmount);
void GuiWalletRecoveryDestruct(void *obj, void *param);
void *GuiWalletRecoverySharePhrase(lv_obj_t *parent, uint8_t wordAmount);

#endif /* _GUI_SETTING_WIDGETS_H */

