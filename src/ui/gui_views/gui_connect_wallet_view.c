#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_connect_wallet_widgets.h"
#include "gui_pending_hintbox.h"
#include "gui_lock_widgets.h"

int32_t GuiConnectWalletViewEventProcess(void* self, uint16_t usEvent, void* param, uint16_t usLen)
{
    GUI_ASSERT(g_connectWalletView.isActive);

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_connectWalletView = {
    .id = CONNECT_WALLET,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiConnectWalletViewEventProcess,
};

