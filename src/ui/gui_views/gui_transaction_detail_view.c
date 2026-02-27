#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
// #include "gui_chain.h"
#include "gui_transaction_detail_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_pending_hintbox.h"
#include "gui_attention_hintbox.h"
#include "device_setting.h"

int32_t GuiTransactionDetailViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t viewType = 0;
    switch (usEvent) {
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