#include "gui.h"
#include "gui_views.h"
#include "gui_create_multisig_wallet_widgets.h"

int32_t GuiCreateMultiViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint32_t rcvValue;
    GUI_ASSERT(g_createMultisigWalletView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiCreateMultiInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiCreateMultiDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiCreateMultiRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiCreateMultiPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiCreateMultiNextTile(0xFF);
        break;
    case SIG_INIT_SDCARD_CHANGE:
        ListMicroCardXpubFile();
        return ERR_GUI_UNHANDLED;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_createMultisigWalletView = {
    .id = SCREEN_CREATE_MULTI,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiCreateMultiViewEventProcess,
};

