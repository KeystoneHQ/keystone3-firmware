#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "screen_manager.h"
#include "gui_lock_device_widgets.h"
#include "gui_lock_widgets.h"

static void *pageParam = NULL;

static bool IsLockTimePage()
{
    return pageParam != NULL;
}

static int32_t GuiLockDeviceViewInit(void *param)
{
    return SUCCESS_CODE;
}

static int32_t GuiLockDeviceViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiLockDeviceViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_lockDeviceView = {
    .id = SCREEN_LOCK_DEVICE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiLockDeviceViewEventProcess,
};

