#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_transaction_detail_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_pending_hintbox.h"
#include "gui_attention_hintbox.h"
#include "device_setting.h"

int32_t GuiTransactionDetailViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t viewType = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            viewType = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiTransactionDetailInit(viewType);
        if (supportBlindSigning(viewType) && GetEnableBlindSigning() == false) {

            // if not enable blind signing, we show hintbox to notify user , this tx may be sensitive
            GuiCreateEnableBlindSigningHintbox();
        }
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiTransactionDetailDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiTransactionDetailRefresh();
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiTransactionDetailVerifyPasswordSuccess();
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *) passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiLockScreenErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiLockScreenPassCode(false);
        GuiSignVerifyPasswordErrorCount(param);
        break;
    case SIG_FINGER_RECOGNIZE_RESPONSE:
        GuiSignDealFingerRecognize(param);
        break;
    case SIG_TRANSACTION_PARSE_SUCCESS:
        GuiTransactionDetailParseSuccess(param);
        break;
    case SIG_TRANSACTION_PARSE_FAIL:
        GuiTransactionParseFailed();
        break;
    case SIG_HIDE_TRANSACTION_LOADING:
    case SIG_HIDE_TRANSACTION_PARSE_LOADING_DELAY:
        GuiPendingHintBoxRemove();
        break;
    case SIG_SHOW_TRANSACTION_LOADING_DELAY:
        EthContractCheckRawDataCallback();
        break;
#ifndef BTC_ONLY
    case SIG_INIT_PULLOUT_USB:
        GuiTransactionUsbPullout();
        break;
#endif
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_transactionDetailView = {
    .id = SCREEN_TRANSACTION_DETAIL,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiTransactionDetailViewEventProcess,
};