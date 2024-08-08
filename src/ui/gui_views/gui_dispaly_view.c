#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_display_widgets.h"

int32_t GuiDisplayViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiDisplayWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiDisplayWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiDisplayWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiDisplayWidgetsRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_displayView = {
    .id = SCREEN_DISPLAY,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiDisplayViewEventProcess,
};
