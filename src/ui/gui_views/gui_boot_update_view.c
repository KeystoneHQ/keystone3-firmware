#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_boot_update_widgets.h"

int32_t GuiBootUpdateViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiBootUpdateInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiBootUpdateDeInit();
        break;
    case SIG_BOOT_UPDATE_SUCCESS:
        GuiBootUpdateSuccess();
        break;
    case SIG_BOOT_UPDATE_FAIL:
        // GuiBootUpdateFail();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_bootUpdateView = {
    .id = SCREEN_BOOT_UPDATE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiBootUpdateViewEventProcess,
};
