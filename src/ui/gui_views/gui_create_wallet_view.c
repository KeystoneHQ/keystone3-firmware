#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_create_wallet_widgets.h"

static int32_t GuiCreateWalletViewInit(uint8_t walletMethod)
{
    GuiEnterPassLabelRefresh();
    GuiCreateWalletInit(walletMethod);
    return SUCCESS_CODE;
}

static int32_t GuiCreateWalletViewDeInit(void)
{
    GuiCreateWalletDeInit();
    return SUCCESS_CODE;
}

int32_t GuiCreateWalletViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t walletMethod = 0;
    GUI_ASSERT(g_createWalletView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            walletMethod = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiCreateWalletViewInit(walletMethod);
    case GUI_EVENT_OBJ_DEINIT:
        GuiCreateWalletDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiCreateWalletRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiCreateWalletPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiCreateWalletNextTile();
        break;
    case SIG_SETTING_SET_PIN:
        GuiCreateWalletSetPinPass((const char *)param);
        break;
    case SIG_SETTING_REPEAT_PIN:
        GuiCreateWalletRepeatPinPass((const char *)param);
        break;
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

