#include "gui.h"
#include "gui_views.h"
#include "gui_obj.h"
#include "gui_zcash_verify_widgets.h"

static int32_t GuiZcashVerifyViewInit(void)
{
    GuiZcashVerifyWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiZcashVerifyViewDeInit(void)
{
    GuiZcashVerifyWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiZcashVerifyViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiZcashVerifyViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiZcashVerifyViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiZcashVerifyWidgetsRefresh();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_zcashVerifyView = {
    .id = SCREEN_ZCASH_VERIFY,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiZcashVerifyViewEventProcess,
};
