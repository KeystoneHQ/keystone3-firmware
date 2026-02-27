#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_standard_receive_widgets.h"
#include "gui_pending_hintbox.h"
#include "gui_model.h"

static int32_t GuiStandardReceiveViewInit(uint8_t chain)
{
    return SUCCESS_CODE;
}

static int32_t GuiStandardReceiveViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiStandardReceiveViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t chain = 0;
    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_standardReceiveView = {
    .id = SCREEN_STANDARD_RECEIVE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiStandardReceiveViewEventProcess,
};
