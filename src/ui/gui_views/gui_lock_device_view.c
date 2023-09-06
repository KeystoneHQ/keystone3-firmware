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
    pageParam = param;
    if (!IsLockTimePage()) {
        SetPageLockScreen(false);
    }
    GuiLockDeviceInit(param);
    return SUCCESS_CODE;
}

static int32_t GuiLockDeviceViewDeInit(void)
{
    GuiLockDeviceDeInit();
    if (!IsLockTimePage()) {
        SetPageLockScreen(true);
    }
    pageParam =  NULL;
    return SUCCESS_CODE;
}

int32_t GuiLockDeviceViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiLockDeviceViewInit(param);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiLockDeviceViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiLockDeviceRefresh();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    case SIG_SETTING_DEL_WALLET_PASS_SETUP:
        GuiDelALLWalletSetup();
        break;
    case SIG_LOCK_VIEW_SCREEN_CLEAR_ALL_TOP:
        GuiClearAllTop();
        break;
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

