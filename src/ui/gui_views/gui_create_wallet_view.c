#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_create_wallet_widgets.h"

static int32_t GuiCreateWalletViewInit(uint8_t walletMethod)
{
    return SUCCESS_CODE;
}

int32_t GuiCreateWalletViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t walletMethod = 0;
    GUI_ASSERT(g_createWalletView.isActive);

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_createWalletView = {
    .id = CREATE_WALLET,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiCreateWalletViewEventProcess,
};

