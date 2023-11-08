#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_transaction_signature_widgets.h"
#include "gui_lock_widgets.h"

static int32_t GuiTransactionSignatureViewInit(uint8_t viewType)
{
    GuiTransactionSignatureInit(viewType);
    return SUCCESS_CODE;
}

static int32_t GuiTransactionSignatureViewDeInit(void)
{
    GuiTransactionSignatureDeInit();
    return SUCCESS_CODE;
}

int32_t GuiTransactionSignatureViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t viewType = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            viewType = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiTransactionSignatureViewInit(viewType);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiTransactionSignatureViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiTransactionSignatureRefresh();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiTransactionSignatureHandleURGenerate((char*)param, usLen);
        break;
    case SIG_BACKGROUND_UR_UPDATE:
        GuiTransactionSignatureHandleURUpdate((char*)param, usLen);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_transactionSignatureView = {
    .id = SCREEN_TRANSACTION_SIGNATURE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiTransactionSignatureViewEventProcess,
};