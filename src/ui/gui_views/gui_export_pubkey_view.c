#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_export_pubkey_widgets.h"

static int32_t GuiExportPubkeyViewInit(uint8_t chain)
{
    GuiExportPubkeyInit(chain);
    return SUCCESS_CODE;
}

static int32_t GuiExportPubkeyViewDeInit(void)
{
    GuiExportPubkeyDeInit();
    return SUCCESS_CODE;
}

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
        return GuiExportPubkeyViewInit(chain);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiExportPubkeyViewDeInit();
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
