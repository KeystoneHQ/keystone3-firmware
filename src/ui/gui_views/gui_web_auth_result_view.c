#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_web_auth_result_widgets.h"
#include "err_code.h"

static int32_t GuiWebAuthResultViewInit()
{
    GuiWebAuthResultAreaInit();
    return SUCCESS_CODE;
}

static int32_t GuiWebAuthResultViewDeInit(void)
{
    GuiWebAuthResultAreaDeInit();
    return SUCCESS_CODE;
}

int32_t GuiWebAuthResultViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_webAuthResultView.isActive);
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiWebAuthResultViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiWebAuthResultViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiWebAuthResultAreaRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiWebAuthResultAreaRestart();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiWebAuthResultPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiWebAuthResultNextTile();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    case SIG_WEB_AUTH_CODE_SUCCESS:
        GuiWebAuthShowAuthCode(param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_webAuthResultView = {
    .id = SCREEN_WEB_AUTH_RESULT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiWebAuthResultViewEventProcess,
};

