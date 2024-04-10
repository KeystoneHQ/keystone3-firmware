#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_export_pubkey_widgets.h"

int32_t GuiExportPubkeyViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t chain = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            chain = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiExportPubkeyInit(chain);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiExportPubkeyDeInit();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_exportPubkeyView = {
    .id = SCREEN_EXPORT_PUBKEY,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiExportPubkeyViewEventProcess,
};
