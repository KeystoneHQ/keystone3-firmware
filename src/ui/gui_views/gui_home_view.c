#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_home_widgets.h"

static int32_t GuiHomeViewInit(void)
{
    GuiHomeAreaInit();
    GuiModeGetWalletDesc();
    return SUCCESS_CODE;
}

static int32_t GuiHomeViewDeInit(void)
{
    GuiHomeDeInit();
    return SUCCESS_CODE;
}

int32_t GuiHomeViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiHomeViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiHomeViewDeInit();
    case GUI_EVENT_DISACTIVE:
        GuiHomeDisActive();
        break;
    case GUI_EVENT_RESTART:
        GuiHomeRestart();
        break;
    case GUI_EVENT_REFRESH:
        GuiHomeRefresh();
        if (param != NULL) {
            GuiModeGetWalletDesc();
        }
        break;
    case SIG_INIT_GET_CURRENT_WALLET_DESC:
        GuiHomeSetWalletDesc((WalletDesc_t *)param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_homeView = {
    .id = SCREEN_HOME,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiHomeViewEventProcess,
};
