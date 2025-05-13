#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_multi_accounts_receive_widgets.h"

int32_t GuiMultiAccountsReceiveViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t chain = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            chain = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiMultiAccountsReceiveInit(chain);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiMultiAccountsReceiveDeInit();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    case GUI_EVENT_REFRESH:
        // GuiMultiAccountsReceiveRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiMultiAccountsReceivePrevTile();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_multiAccountsReceiveView = {
    .id = SCREEN_MULTI_ACCOUNTS_RECEIVE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiMultiAccountsReceiveViewEventProcess,
};
