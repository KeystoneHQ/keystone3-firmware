#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_about_keystone_widgets.h"

static int32_t GuiAboutKeystoneViewInit(void)
{
    GuiAboutKeystoneWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiAboutKeystoneViewDeInit(void)
{
    GuiAboutKeystoneWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiAboutKeystoneViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_aboutKeystoneView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiAboutKeystoneViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiAboutKeystoneViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiAboutKeystoneWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiAboutKeystoneWidgetsRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_aboutKeystoneView = {
    .id = SCREEN_ABOUT_KEYSTONE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiAboutKeystoneViewEventProcess,
};
