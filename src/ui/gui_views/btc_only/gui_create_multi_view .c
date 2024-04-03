#include "gui.h"
#include "gui_views.h"
#include "gui_create_multi_widgets.h"

int32_t GuiCreateMultiViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_createMultiView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiCreateMultiInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiCreateMultiDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiCreateMultiRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiCreateMultiPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiCreateMultiNextTile(0xFF);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        GuiCreateMultiNextTile(0xFF);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_createMultiView = {
    .id = SCREEN_CREATE_MULTI,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiCreateMultiViewEventProcess,
};

