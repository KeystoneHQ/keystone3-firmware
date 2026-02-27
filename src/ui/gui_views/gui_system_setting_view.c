#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_system_setting_widgets.h"
#include "gui_enter_passcode.h"
#include "gui_lock_widgets.h"
#include "gui_model.h"
#include "gui_enter_passcode.h"

int32_t GuiSystemSettingViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_systemSettingView.isActive);

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_systemSettingView = {
    .id = SCREEN_SYSTEM_SETTING,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSystemSettingViewEventProcess,
};
