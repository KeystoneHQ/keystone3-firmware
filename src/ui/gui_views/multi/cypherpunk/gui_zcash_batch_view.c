#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_zcash_batch_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_pending_hintbox.h"

static int32_t GuiZcashBatchViewInit(void)
{
    GuiZcashBatchWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiZcashBatchViewDeInit(void)
{
    GuiZcashBatchWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiZcashBatchViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiZcashBatchViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiZcashBatchViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiZcashBatchWidgetsRefresh();
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiZcashBatchWidgetsVerifyPasswordSuccess();
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *)passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiLockScreenErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiLockScreenPassCode(false);
        GuiZcashBatchWidgetsSignVerifyPasswordErrorCount(param);
        break;
    case SIG_INIT_PULLOUT_USB:
        GuiZcashBatchWidgetsUsbPullout();
        break;
    case SIG_TRANSACTION_PARSE_SUCCESS:
        GuiZcashBatchWidgetsTransactionParseSuccess();
        break;
    case SIG_TRANSACTION_PARSE_FAIL:
        GuiZcashBatchWidgetsTransactionParseFail();
        break;
    case SIG_HIDE_TRANSACTION_LOADING:
        GuiPendingHintBoxRemove();
        break;
    case SIG_SHOW_TRANSACTION_LOADING:
        GuiNoPendingHintBoxOpen(_("Loading"));
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_zcashBatchView = {
    .id = SCREEN_ZCASH_BATCH_TX,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiZcashBatchViewEventProcess,
};
