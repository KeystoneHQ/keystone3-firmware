#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_multisig_transaction_signature_widgets.h"
#include "gui_lock_widgets.h"

static int32_t GuiMultisigTransactionSignatureViewInit(void)
{
    GuiMultisigTransactionSignaureWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiMultisigTransactionSignatureViewDeInit(void)
{
    GuiMultisigTransactionSignaureWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiMultisigTransactionSignatureViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t viewType = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiMultisigTransactionSignatureViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiMultisigTransactionSignatureViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiMultisigTransactionSignaureWidgetsRefresh();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiAnimantingQRCodeFirstUpdate((char*)param, usLen);
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