#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_update_success_widgets.h"

int32_t GuiUpdateSuccessViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiUpdateSuccessInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_updateSuccessView = {
    .id = SCREEN_DEVICE_UPDATE_SUCCESS,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiUpdateSuccessViewEventProcess,
};
