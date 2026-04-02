#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_multisig_transaction_signature_widgets.h"
#include "gui_lock_widgets.h"

int32_t GuiMultisigTransactionSignatureViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiMultisigTransactionSignaureWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiMultisigTransactionSignaureWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiMultisigTransactionSignaureWidgetsRefresh();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiAnimantingQRCodeFirstUpdate((char*)param, usLen);
        UpdateDoneBtnState();
        break;
    case SIG_BACKGROUND_UR_UPDATE:
        GuiAnimatingQRCodeUpdate((char*)param, usLen);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_multisigTransactionSignatureView = {
    .id = SCREEN_MULTISIG_TRANSACTION_SIGNATURE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiMultisigTransactionSignatureViewEventProcess,
};