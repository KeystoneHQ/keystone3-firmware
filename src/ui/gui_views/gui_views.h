#ifndef _GUI_VIEWS_H
#define _GUI_VIEWS_H

#include "gui_framework.h"
#include "ui_display_task.h"

typedef enum {
    SIG_INIT_VIEW_START = GUI_EVENT_FRAME_WORK_RSV + 1, //101
    SIG_INIT_GET_ACCOUNT_NUMBER,
    SIG_INIT_GET_CURRENT_WALLET_DESC,
    SIG_INIT_BATTERY,
    SIG_INIT_USB_CONNECTION,
    SIG_INIT_USB_STATE_CHANGE,
    SIG_INIT_SDCARD_CHANGE,
    SIG_INIT_FIRMWARE_UPDATE_DENY,
    SIG_INIT_LOW_BATTERY,
    SIG_INIT_POWER_OPTION,
    SIG_INIT_FIRMWARE_PROCESS,
    SIG_INIT_CLOSE_CURRENT_MSG_BOX,
    SIG_INIT_SD_CARD_OTA_COPY,
    SIG_INIT_SD_CARD_OTA_COPY_SUCCESS,
    SIG_INIT_SD_CARD_OTA_COPY_FAIL,
    SIG_INIT_FINGER_UPDATE,
    SIG_INIT_VIEW_BUTT,

    SIG_LOCK_VIEW_VERIFY_PIN = SIG_INIT_VIEW_BUTT + 50,
    SIG_LOCK_VIEW_VERIFY_PASSWORD,
    SIG_LOCK_VIEW_SCREEN_ON_VERIFY,
    SIG_LOCK_VIEW_SCREEN_OFF,
    SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS,
    SIG_LOCK_VIEW_SCREEN_GO_LOCK_DEVICE_PASS,
    SIG_LOCK_VIEW_SCREEN_CLEAR_ALL_TOP,
    SIG_PASSCODE_SWITCH_TO_PIN,
    SIG_PASSCODE_SWITCH_TO_PASSWORD,
    SIG_LOCK_VIEW_BUTT,

    SIG_SETUP_VIEW_START = SIG_LOCK_VIEW_BUTT + 50,
    SIG_SETUP_VIEW_TILE_PREV,
    SIG_SETUP_VIEW_TILE_NEXT,
    SIG_SETUP_VIEW_TILE_UP,
    SIG_SETUP_VIEW_TILE_DOWN,
    SIG_SETUP_VIEW_BUTT,

    SIG_CREATE_SHARE_VIEW_NEXT_SLICE = SIG_SETUP_VIEW_BUTT + 50,
    SIG_CREATE_SHARE_UPDATE_MNEMONIC,
    SIG_CREATE_SHARE_VIEW_BUTT,

    SIG_IMPORT_SHARE_VIEW_NEXT_SLICE = SIG_CREATE_SHARE_VIEW_BUTT + 100,
    SIG_IMPORT_SHARE_VIEW_BUTT,

    SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC = SIG_IMPORT_SHARE_VIEW_BUTT + 100,
    SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS,
    SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL,

    SIG_CREATE_SINGLE_PHRASE_WRITESE_PASS,
    SIG_CREATE_SINGLE_PHRASE_WRITESE_FAIL,
    SIG_CREATE_SINGLE_PHRASE_BUTT,

    SIG_QRCODE_VIEW_SCAN_PASS = SIG_CREATE_SINGLE_PHRASE_BUTT + 50,
    SIG_QRCODE_VIEW_SCAN_FAIL,
    SIG_QRCODE_VIEW_SCAN_BUTT,

    SIG_VERIFY_PASSWORD_PASS = SIG_QRCODE_VIEW_SCAN_BUTT + 500,
    SIG_VERIFY_PASSWORD_FAIL,
    SIG_VERIFY_FINGER_PASS,
    SIG_VERIFY_FINGER_FAIL,
    SIG_SETTING_SET_PIN,
    SIG_SETTING_REPEAT_PIN,
    SIG_SETTING_SET_PASSWORD,
    SIG_SETTING_REPEAT_PASSWORD,
    SIG_SETTING_PASSWORD_RESETTING,
    SIG_SETTING_PASSWORD_RESET_PASS,
    SIG_SETTING_PASSWORD_RESET_FAILED,
    SIG_SETTING_CHANGE_WALLET_DESC_PASS,
    SIG_SETTING_DEL_WALLET_PASS,
    SIG_SETTING_DEL_WALLET_FAIL,
    SIG_SETTING_DEL_WALLET_PASS_SETUP,
    SIG_SETTING_WRITE_PASSPHRASE_PASS,
    SIG_SETTING_WRITE_PASSPHRASE_FAIL,
    SIG_SETTING_CHANGE_PASSWORD_PASS,
    SIG_SETTING_CHANGE_PASSWORD_FAIL,
    SIG_SETTING_VERIFY_PASSWORD,
    SIG_SETTING_ADD_WALLET_AMOUNT_LIMIT,
    SIG_SETTING_ADD_WALLET_CREATE_OR_IMPORT,
    SIG_SETTING_LOG_EXPORT_SUCCESS,
    SIG_SETTING_LOG_EXPORT_FAIL,
    SIG_SETTING_LOG_EXPORT_NOT_ENOUGH_SPACE,
    SIG_SETTING_BUTT,

    SIG_FINGER_REGISTER_STEP_SUCCESS = SIG_SETTING_BUTT + 50,
    SIG_FINGER_REGISTER_STEP_FAIL,
    SIG_FINGER_REGISTER_EXCEEDED_TIMES,
    SIG_FINGER_DELETE_SUCCESS,
    SIG_FINGER_RECOGNIZE_RESPONSE,
    SIG_FINGER_SET_BUTT,

    SIG_FORGET_PASSWORD_SUCCESS = SIG_FINGER_SET_BUTT + 50,
    SIG_FORGET_PASSWORD_FAIL,

    SIG_WEB_AUTH_CODE_SUCCESS = SIG_FORGET_PASSWORD_FAIL + 50,
    SIG_BACKGROUND_UR_GENERATE_SUCCESS,
    SIG_BACKGROUND_UR_UPDATE,
} GUI_VIEW_SIG_ENUM;

extern GUI_VIEW g_initView;
extern GUI_VIEW g_lockView;
extern GUI_VIEW g_homeView;
extern GUI_VIEW g_setupView;
extern GUI_VIEW g_createWalletView;
extern GUI_VIEW g_singlePhraseView;
extern GUI_VIEW g_importPhraseView;
extern GUI_VIEW g_createShareView;
extern GUI_VIEW g_importShareView;
extern GUI_VIEW g_settingView;
extern GUI_VIEW g_connectWalletView;
extern GUI_VIEW g_qrCodeView;
extern GUI_VIEW g_passphraseView;
extern GUI_VIEW g_utxoReceiveView;
extern GUI_VIEW g_ethereumReceiveView;
extern GUI_VIEW g_standardReceiveView;
extern GUI_VIEW g_forgetPassView;
extern GUI_VIEW g_lockDeviceView;
extern GUI_VIEW g_firmwareUpdateView;
extern GUI_VIEW g_webAuthView;
extern GUI_VIEW g_webAuthResultView;
extern GUI_VIEW g_systemSettingView;
extern GUI_VIEW g_purposeView;
extern GUI_VIEW g_aboutView;
extern GUI_VIEW g_aboutKeystoneView;
extern GUI_VIEW g_aboutInfoView;
extern GUI_VIEW g_aboutTermsView;
extern GUI_VIEW g_wipeDeviceView;
extern GUI_VIEW g_walletTutorialView;
extern GUI_VIEW g_selfDestructView;
extern GUI_VIEW g_inactiveView;
extern GUI_VIEW g_displayView;
extern GUI_VIEW g_tutorialView;
extern GUI_VIEW g_connectionView;
extern GUI_VIEW g_DevicePublicKeyView;

void OpenImportWalletHandler(lv_event_t *e);
void OpenCreateWalletHandler(lv_event_t *e);
void OpenViewHandler(lv_event_t *e);
void CloseTimerCurrentViewHandler(lv_event_t *e);
void ReturnHandler(lv_event_t *e);
void NextTileHandler(lv_event_t *e);
void CloseCurrentParentHandler(lv_event_t *e);
void CloseCurrentUserDataHandler(lv_event_t *e);
void ToggleSwitchBoxHandler(lv_event_t *e);
void GuiWriteSeWidget(lv_obj_t *parent);
void GuiWriteSeResult(bool en, int32_t errorCode);
void GuiWriteSeLowPower(void);
void CloseWaringPageHandler(lv_event_t *e);
void CloseCurrentViewHandler(lv_event_t *e);
void CloseCurrentParentAndCloseViewHandler(lv_event_t *e);
void CloseToTargetTileView(uint8_t currentIndex, uint8_t targetIndex);
void CloseParentAndNextHandler(lv_event_t *e);
void GuiViewHintBoxClear(void);

#endif /* _GUI_VIEWS_H */

