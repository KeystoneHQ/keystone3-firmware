#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_setting_widgets.h"
#include "gui_enter_passcode.h"
#include "gui_lock_widgets.h"
#include "gui_qr_code.h"

static int32_t GuiSettingViewInit(void)
{
    GuiSettingInit();
    return SUCCESS_CODE;
}

static int32_t GuiSettingViewDeInit(void)
{
    GuiSettingDeInit();
    return SUCCESS_CODE;
}

int32_t GuiSettingViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint16_t tileIndex = 0;
    uint8_t walletCnt = 0;
    // GUI_ASSERT(g_settingView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiSettingViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiSettingViewDeInit();
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
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *) passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiLockScreenErrorCount(param);
                return SUCCESS_CODE;
            } else {
                tileIndex = sig;
            }
        } else {
            return ERR_GUI_ERROR;
        }
        GuiDevSettingPassCode(false, tileIndex);
        GuiVerifyCurrentPasswordErrorCount(param);
        break;
    case SIG_SETTING_SET_PIN:
        GuiSettingSetPinPass((const char *)param);
        break;
    case SIG_SETTING_REPEAT_PIN:
        GuiSettingRepeatPinPass((const char *)param);
        break;
    case SIG_SETTING_PASSWORD_RESET_PASS:
        GuiResettingPassWordSuccess();
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
    case SIG_SETTING_CHANGE_PASSWORD_PASS:
        GuiChangePassWord(true);
        break;
    case SIG_SETTING_CHANGE_PASSWORD_FAIL:
        GuiChangePassWord(false);
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
    case SIG_SETTING_ADD_WALLET_CREATE_OR_IMPORT:
        if (param != NULL) {
            walletCnt = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiAddWalletCreateOrImport(walletCnt);
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
        GuiSettingFingerDeleteSuccess();
        break;
    case SIG_FINGER_RECOGNIZE_RESPONSE:
        GuiSettingDealFingerRecognize(param);
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

