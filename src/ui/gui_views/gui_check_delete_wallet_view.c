#ifndef WEB3_VERSION
#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_check_delete_wallet_widgets.h"
#include "gui_setting_widgets.h"

int32_t GuiCheckDeleteWalletViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiCheckDeleteWalletInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiCheckDeleteWalletDeInit();
        break;
    case SIG_SETTING_DEL_WALLET_PASS:
        GuiDelWallet(true);
        break;
    case SIG_SETTING_DEL_WALLET_PASS_SETUP:
        GuiDelWalletToSetup();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_checkDeleteWalletView = {
    .id = SCREEN_DEVICE_UPDATE_SUCCESS,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiCheckDeleteWalletViewEventProcess,
};

#endif