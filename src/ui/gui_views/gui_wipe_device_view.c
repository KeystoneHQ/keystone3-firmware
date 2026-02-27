#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_wipe_device_widgets.h"

static int32_t GuiWipeDeviceViewInit(void)
{
    GuiWipeDeviceWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiWipeDeviceViewDeInit(void)
{
    GuiWipeDeviceWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiWipeDeviceViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_wipeDeviceView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiWipeDeviceViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiWipeDeviceViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiWipeDeviceWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiWipeDeviceWidgetsRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_wipeDeviceView = {
    .id = SCREEN_WIPE_DEVICE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiWipeDeviceViewEventProcess,
};
