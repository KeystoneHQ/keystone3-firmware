#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_purpose_widgets.h"

static int32_t GuiPurposeViewInit(void)
{
    return SUCCESS_CODE;
}

static int32_t GuiPurposeViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiPurposeViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_purposeView.isActive);

    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_purposeView = {
    .id = SCREEN_PURPOSE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiPurposeViewEventProcess,
};

