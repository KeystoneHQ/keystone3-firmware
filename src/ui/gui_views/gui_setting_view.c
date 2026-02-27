#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_setting_widgets.h"
#include "gui_enter_passcode.h"
#include "gui_lock_widgets.h"
// #include "gui_qr_code.h"

int32_t GuiSettingViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint16_t tileIndex = 0;
    uint8_t walletCnt = 0;
    // GUI_ASSERT(g_settingView.isActive);

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_settingView = {
    .id = SCREEN_SETTING,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSettingViewEventProcess,
};

