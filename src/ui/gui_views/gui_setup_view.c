#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_setup_widgets.h"

static int32_t GuiSetupViewInit(void)
{
    GuiSetupAreaInit();
    return SUCCESS_CODE;
}

static int32_t GuiSetupViewDeInit(void)
{
    GuiSetupAreaDeInit();
    return SUCCESS_CODE;
}

int32_t GuiSetupViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_setupView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiSetupViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiSetupViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiSetupAreaRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiSetupAreaRestart();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiSetupPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiSetupNextTile();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_setupView = {
    .id = SCREEN_SETUP,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSetupViewEventProcess,
};

