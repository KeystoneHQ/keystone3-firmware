#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_forget_pass_widgets.h"
#include "gui_hintbox.h"
#include "gui_lock_widgets.h"

int32_t GuiForgetViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint16_t tileIndex = 0;
    int32_t ret = SUCCESS_CODE;
    // GUI_ASSERT(g_settingView.isActive);

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_forgetPassView = {
    .id = SCREEN_FORGET_PASSCODE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiForgetViewEventProcess,
};

