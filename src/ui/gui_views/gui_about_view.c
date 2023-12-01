#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_about_widgets.h"
#include "gui_about_info_widgets.h"

static int32_t GuiAboutViewInit(void)
{
    GuiAboutWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiAboutViewDeInit(void)
{
    GuiAboutWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiAboutViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_aboutView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiAboutViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiAboutViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiAboutWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiAboutWidgetsRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_aboutView = {
    .id = SCREEN_ABOUT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiAboutViewEventProcess,
};
