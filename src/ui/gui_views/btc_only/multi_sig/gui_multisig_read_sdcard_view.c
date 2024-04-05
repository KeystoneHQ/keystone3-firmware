#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_multisig_read_sdcard_widgets.h"
#include "gui.h"

int32_t GuiMultisigReadSdcardViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_multisigReadSdcardView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiMultisigReadSdcardWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiMultisigReadSdcardWidgetsDeInit();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_multisigReadSdcardView = {
    .id = SCREEN_MULTISIG_READ_SDCARD,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiMultisigReadSdcardViewEventProcess,
};
