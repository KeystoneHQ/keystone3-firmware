#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_multisig_select_import_method_widgets.h"
#include "gui.h"

int32_t GuiMultisigSelectImportMethodViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_multisigSelectImportMethodView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiMultisigSelectImportMethodWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiMultisigSelectImportMethodWidgetsDeInit();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_multisigSelectImportMethodView = {
    .id = SCREEN_MULTISIG_SELECT_IMPORT_METHOD,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiMultisigSelectImportMethodViewEventProcess,
};
