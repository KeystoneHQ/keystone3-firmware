#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_scan_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_pending_hintbox.h"
static int32_t GuiScanViewInit(void)
{
    GuiScanInit();
    return SUCCESS_CODE;
}

static int32_t GuiScanViewDeInit(void)
{
    GuiScanDeInit();
    return SUCCESS_CODE;
}

int32_t GuiScanViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiScanViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiScanViewDeInit();
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
        GuiTransactionCheckPass();
        break;
    case SIG_TRANSACTION_CHECK_FAIL:
        GuiTransactionCheckFiald((PtrT_TransactionCheckResult)param);
        break;
    case SIG_SHOW_CHECKING_LAODING:
        GuiPendingHintBoxOpen(_("Loading"), "");
        break;
    case SIG_HIDE_CHECKING_LAODING:
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

