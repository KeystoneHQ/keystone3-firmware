#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_ethereum_receive_widgets.h"

static int32_t GuiEthereumReceiveViewInit(void)
{
    GuiEthereumReceiveInit();
    return SUCCESS_CODE;
}

static int32_t GuiEthReceiveViewDeInit(void)
{
    printf("GuiEthereumReceiveViewDeInit\r\n");
    GuiEthereumReceiveDeInit();
    return SUCCESS_CODE;
}

int32_t GuiEthereumReceiveViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiEthereumReceiveViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiEthReceiveViewDeInit();
    case GUI_EVENT_DISACTIVE:
        break;
    case GUI_EVENT_REFRESH:
        GuiEthereumReceiveRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiEthereumReceivePrevTile();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_ethereumReceiveView = {
    .id = SCREEN_ETHEREUM_RECEIVE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiEthereumReceiveViewEventProcess,
};
