#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_lock_widgets.h"
#include "keystore.h"
#include "fingerprint_process.h"
#include "gui_firmware_update_widgets.h"
#include "gui_animating_qrcode.h"
#include "gui_status_bar.h"
#include "account_manager.h"
#include "gui_lock_device_widgets.h"
#include "gui_api.h"
#include "usb_task.h"
#include "device_setting.h"
#include "drv_aw32001.h"
#include "lv_i18n_api.h"
#include "gui_model.h"

int32_t GuiLockViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint32_t rcvValue;

    switch (usEvent) {
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

