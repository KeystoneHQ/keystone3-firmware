#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_zcash_verify_result_widgets.h"

int32_t GuiZcashVerifyResultViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiZcashVerifyResultWidgetsInit((const char *)param);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiZcashVerifyResultWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiZcashVerifyResultWidgetsRefresh();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_zcashVerifyResultView = {
    .id = SCREEN_ZCASH_VERIFY_RESULT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiZcashVerifyResultViewEventProcess,
};
