#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_firmware_update_widgets.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_setup_done_widgets.h"

int32_t GuiSetupDoneViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint32_t rcvValue;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiSetupDoneInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiSetupDoneDeInit();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    case SIG_INIT_SDCARD_CHANGE:
        rcvValue = *(uint32_t *)param;
        GuiStatusBarSetSdCard(!rcvValue, false);
    case GUI_EVENT_REFRESH:
        GuiSetupDoneRefresh();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_setupDoneView = {
    .id = SCREEN_SETUP_DONE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSetupDoneViewEventProcess,
};

