#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_create_share_widgets.h"

int32_t GuiCreateShareViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    int32_t ret = SUCCESS_CODE;
    GUI_ASSERT(g_createShareView.isActive);
    uint8_t entropyMethod;

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_createShareView = {
    .id = CREATE_SHARE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiCreateShareViewEventProcess,
};

