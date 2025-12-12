#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_import_share_widgets.h"

static int32_t GuiImportShareViewInit(uint8_t wordsCnt)
{
    return SUCCESS_CODE;
}

static int32_t GuiImportShareViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiImportShareViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t wordsCnt = 0;
    int32_t ret = SUCCESS_CODE;
    GUI_ASSERT(g_importShareView.isActive);

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_importShareView = {
    .id = IMPORT_SHARE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiImportShareViewEventProcess,
};

