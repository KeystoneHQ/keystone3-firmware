#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_connection_widgets.h"

static int32_t GuiConnectionViewInit(void)
{
    GuiConnectionWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiConnectionViewDeInit(void)
{
    GuiConnectionWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiConnectionViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiConnectionViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiConnectionViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiConnectionWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiConnectionWidgetsRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_connectionView = {
    .id = SCREEN_CONNECTION,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiConnectionViewEventProcess,
};
