#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_scan_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_pending_hintbox.h"

int32_t GuiScanViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiScanInit(param, usLen);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiScanDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiScanRefresh();
        break;
    case SIG_QRCODE_VIEW_SCAN_FAIL:
        GuiScanResult(false, param);
        break;
    case SIG_QRCODE_VIEW_SCAN_PASS:
        GuiScanResult(true, param);
        break;
    case SIG_TRANSACTION_CHECK_PASS:
        printf("chekx pass");
        GuiTransactionCheckPass();
        break;
    case SIG_TRANSACTION_CHECK_FAIL:
        GuiTransactionCheckFailed((PtrT_TransactionCheckResult)param);
        break;
    case SIG_SHOW_TRANSACTION_LOADING:
        // GuiPendingHintBoxOpen(_("Loading"), "");
        // todo add pending anim
        GuiNoPendingHintBoxOpen(_("Loading"));
        break;
    case SIG_HIDE_TRANSACTION_LOADING:
        GuiPendingHintBoxRemove();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_scanView = {
    .id = SCREEN_SCAN,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiScanViewEventProcess,
};
