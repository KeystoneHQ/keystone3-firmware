#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_transaction_signature_widgets.h"
#include "gui_lock_widgets.h"

static int32_t GuiTransactionSignatureViewInit(uint8_t viewType)
{
    return SUCCESS_CODE;
}

static int32_t GuiTransactionSignatureViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiTransactionSignatureViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t viewType = 0;
    switch (usEvent) {
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