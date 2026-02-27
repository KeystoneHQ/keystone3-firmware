#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_device_public_key_widgets.h"

static int32_t GuiDevicePublicKeyViewInit(void)
{
    GuiDevicePublicKeyWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiDevicePublicKeyViewDeInit(void)
{
    GuiDevicePublicKeyWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiDevicePublicKeyViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiDevicePublicKeyViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiDevicePublicKeyViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiDevicePublicKeyWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiDevicePublicKeyWidgetsRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_DevicePublicKeyView = {
    .id = SCREEN_DEVICE_PUB_KEY,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiDevicePublicKeyViewEventProcess,
};
