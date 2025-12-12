#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_eth_batch_tx_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_pending_hintbox.h"

static int32_t GuiEthBatchTxViewInit(void)
{
    return SUCCESS_CODE;
}

static int32_t GuiEthBatchTxViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiEthBatchTxViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_ethBatchTxView = {
    .id = SCREEN_ETH_BATCH_TX,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiEthBatchTxViewEventProcess,
};