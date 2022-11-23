#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_about_terms_widgets.h"

static int32_t GuiAboutTermsViewInit(void)
{
    GuiAboutTermsWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiAboutTermsViewDeInit(void)
{
    GuiAboutTermsWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiAboutTermsViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_aboutTermsView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiAboutTermsViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiAboutTermsViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiAboutTermsWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiAboutTermsWidgetsRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_aboutTermsView = {
    .id = SCREEN_ABOUT_TERMS,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiAboutTermsViewEventProcess,
};
