#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_web_auth_widgets.h"
#include "err_code.h"

static int32_t GuiWebAuthViewInit()
{
    GuiWebAuthAreaInit();
    return SUCCESS_CODE;
}

static int32_t GuiWebAuthViewDeInit(void)
{
    GuiWebAuthAreaDeInit();
    return SUCCESS_CODE;
}

int32_t GuiWebAuthViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_webAuthView.isActive);
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiWebAuthViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiWebAuthViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiWebAuthAreaRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiWebAuthAreaRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_webAuthView = {
    .id = SCREEN_WEB_AUTH,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiWebAuthViewEventProcess,
};

