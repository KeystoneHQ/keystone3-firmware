#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_about_info_widgets.h"

int32_t GuiAboutInfoViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    int32_t ret = SUCCESS_CODE;
    GUI_ASSERT(g_aboutInfoView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiAboutInfoWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiAboutInfoWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiAboutInfoWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiAboutInfoWidgetsRestart();
        break;
    case SIG_SETTING_LOG_EXPORT_SUCCESS:
        GuiAboutWidgetsLogExport(true, 0);
        break;
    case SIG_SETTING_LOG_EXPORT_FAIL:
        if (param != NULL) {
            ret = *(int32_t *)param;
        }
        GuiAboutWidgetsLogExport(false, ret);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_aboutInfoView = {
    .id = SCREEN_ABOUT_INFO,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiAboutInfoViewEventProcess,
};
