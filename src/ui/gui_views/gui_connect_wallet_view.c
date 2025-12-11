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
    case GUI_EVENT_OBJ_INIT:
        GuiConnectWalletInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiConnectWalletDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiConnectWalletRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiConnectWalletPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiConnectWalletNextTile();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiConnectWalletHandleURGenerate((char*)param, usLen);
        break;
    case SIG_BACKGROUND_UR_UPDATE:
        GuiConnectWalletHandleURUpdate((char*)param, usLen);
        break;
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

