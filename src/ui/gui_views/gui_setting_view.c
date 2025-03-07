#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_setting_widgets.h"
#include "gui_enter_passcode.h"
#include "gui_lock_widgets.h"
#include "gui_qr_code.h"

int32_t GuiSettingViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint16_t tileIndex = 0;
    uint8_t walletCnt = 0;
    // GUI_ASSERT(g_settingView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiSettingInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiSettingDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiSettingRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        if (param != NULL) {
            tileIndex = *(uint8_t *)param;
        } else {
            GuiDevSettingPrevTile(0);
        }
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        if (param != NULL) {
            tileIndex = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiDevSettingNextTile(tileIndex);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            } else {
                tileIndex = *(uint16_t *)param;
            }
        } else {
            return ERR_GUI_ERROR;
        }
        //temporary fix sign tx qrcode not animating;
        QRCodeControl(false);
        GuiDevSettingPassCode(true, tileIndex);
        break;
    case SIG_SETTING_WRITE_PASSPHRASE_VERIFY_PASS:
        GuiSettingAnimSetLabel(_("seed_check_wait_verify"));
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *) passwordVerifyResult->signal;
            uint16_t cnt = passwordVerifyResult->errorCount;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiLockScreenErrorCount(param);
                return SUCCESS_CODE;
            } else if (sig == SIG_FINGER_REGISTER_ADD_SUCCESS) {
                if (cnt == 4) {
                    GuiFingerCancelRegister();
                }
            } else {
                tileIndex = sig;
            }
        } else {
            return ERR_GUI_ERROR;
        }
        GuiVerifyCurrentPasswordErrorCount(param);
        break;
    case SIG_SETTING_SET_PIN:
        GuiSettingSetPinPass((const char *)param);
        break;
    case SIG_SETTING_REPEAT_PIN:
        GuiSettingRepeatPinPass((const char *)param);
        break;
    case SIG_SETTING_CHANGE_WALLET_DESC_PASS:
        GuiChangeWalletDesc(true);
        break;
    case SIG_SETTING_DEL_WALLET_PASS:
        GuiDelWallet(true);
        break;
    case SIG_SETTING_WRITE_PASSPHRASE_PASS:
        GuiWritePassphrase(true);
        break;
    case SIG_SETTING_WRITE_PASSPHRASE_FAIL:
        GuiWritePassphrase(false);
        break;
    case SIG_SETTING_CHANGE_PASSWORD_PASS:
        GuiChangePassWord(true);
        break;
    case SIG_SETTING_CHANGE_PASSWORD_FAIL:
        GuiChangePassWord(false);
        if (param != NULL) {
            GuiVerifyCurrentPasswordErrorCount(param);
        }
        break;
    case SIG_SETTING_ADD_WALLET_AMOUNT_LIMIT:
        GuiAddWalletAmountLimit();
        break;
    case SIG_CREATE_SINGLE_PHRASE_WRITESE_PASS:
        GuiWalletRecoveryWriteSe(true);
        break;
    case SIG_CREATE_SINGLE_PHRASE_WRITESE_FAIL:
        GuiWalletRecoveryWriteSe(false);
        break;
    case SIG_SETTING_DEL_WALLET_PASS_SETUP:
        GuiDelWalletSetup();
        break;
    case SIG_INIT_GET_ACCOUNT_NUMBER:
        if (param != NULL) {
            walletCnt = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiAddWalletGetWalletAmount(walletCnt);
        break;
    case SIG_FINGER_REGISTER_STEP_SUCCESS:
        GuiSettingFingerRegisterSuccess(param);
        break;
    case SIG_FINGER_REGISTER_STEP_FAIL:
        GuiSettingFingerRegisterFail(param);
        break;
    case SIG_FINGER_REGISTER_EXCEEDED_TIMES:
        GuiSettingFingerRegisterFail(NULL);
        break;
    case SIG_FINGER_DELETE_SUCCESS:
        GuiDevSettingPrevTile(0);
        break;
    case SIG_FINGER_RECOGNIZE_RESPONSE:
        GuiSettingDealFingerRecognize(param);
        break;
    case GUI_EVENT_UPDATE_KEYBOARD:
        GuiWalletRecoveryUpdateKeyboard();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_settingView = {
    .id = SCREEN_SETTING,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSettingViewEventProcess,
};

