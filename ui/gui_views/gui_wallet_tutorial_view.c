#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_wallet_tutorial_widgets.h"
#include "err_code.h"

static int32_t GuiWalletTutorialViewInit(uint8_t index)
{
    GuiWalletTutorialInit(index);
    return SUCCESS_CODE;
}

static int32_t GuiWalletTutorialViewDeInit(void)
{
    GuiWalletTutorialDeInit();
    return SUCCESS_CODE;
}

int32_t GuiWalletTutorialViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_walletTutorialView.isActive);

    uint8_t index = 0;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            index = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiWalletTutorialViewInit(index);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiWalletTutorialViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiWalletTutorialRefresh();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_walletTutorialView = {
    .id = SCREEN_WALLET_TUTORIAL,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiWalletTutorialViewEventProcess,
};

