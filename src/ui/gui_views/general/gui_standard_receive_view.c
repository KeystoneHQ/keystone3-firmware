#ifndef BTC_ONLY
#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_standard_receive_widgets.h"
#include "gui_model.h"

static int32_t GuiStandardReceiveViewInit(uint8_t chain)
{
    GuiStandardReceiveInit(chain);
    return SUCCESS_CODE;
}

static int32_t GuiStandardReceiveViewDeInit(void)
{
    GuiStandardReceiveDeInit();
    return SUCCESS_CODE;
}

int32_t GuiStandardReceiveViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t chain = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            chain = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiStandardReceiveViewInit(chain);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiStandardReceiveViewDeInit();
    case GUI_EVENT_DISACTIVE:
        break;
    case GUI_EVENT_REFRESH:
        // GuiStandardReceiveRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiStandardReceivePrevTile();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_standardReceiveView = {
    .id = SCREEN_STANDARD_RECEIVE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiStandardReceiveViewEventProcess,
};
#endif
