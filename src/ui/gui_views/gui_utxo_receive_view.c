#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_utxo_receive_widgets.h"
#include "gui_global_resources.h"

static int32_t GuiReceiveViewInit(uint8_t chain)
{
    GuiReceiveInit(chain);
    return SUCCESS_CODE;
}

static int32_t GuiReceiveViewDeInit(void)
{
    GuiReceiveDeInit();
    return SUCCESS_CODE;
}

int32_t GuiReceiveViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t chain = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            chain = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GlobalResourcesInit();
        return GuiReceiveViewInit(chain);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiReceiveViewDeInit();
    case GUI_EVENT_DISACTIVE:
        //GuiBitcoinReceiveDisActive();
        break;
    case GUI_EVENT_REFRESH:
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiReceivePrevTile();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_utxoReceiveView = {
    .id = SCREEN_BITCOIN_RECEIVE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiReceiveViewEventProcess,
};
