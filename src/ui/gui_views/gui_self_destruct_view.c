#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_self_destruct_widgets.h"

static int32_t GuiSelfDestructViewInit(void)
{
    GuiSelfDestructInit();
    return SUCCESS_CODE;
}

static int32_t GuiSelfDestructViewDeInit(void)
{
    GuiSelfDestructDeInit();
    return SUCCESS_CODE;
}

int32_t GuiSelfDestructViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiSelfDestructViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiSelfDestructViewDeInit();
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_selfDestructView = {
    .id = SCREEN_SELF_DESTRUCT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSelfDestructViewEventProcess,
};
