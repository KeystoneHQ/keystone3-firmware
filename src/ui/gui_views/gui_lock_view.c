#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_lock_widgets.h"
#include "keystore.h"
#include "fingerprint_process.h"
#include "account_manager.h"
#include "gui_api.h"
#include "usb_task.h"
#include "device_setting.h"
#include "drv_aw32001.h"

static int32_t GuiLockViewInit(void *param)
{
    GuiLockScreenInit(param);
    return SUCCESS_CODE;
}

static int32_t GuiLockViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiLockViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint32_t rcvValue;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiLockViewInit(param);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiLockViewDeInit();
    case GUI_EVENT_REFRESH:
        GuilockScreenRefresh();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_lockView = {
    .id = SCREEN_LOCK,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiLockViewEventProcess,
};

