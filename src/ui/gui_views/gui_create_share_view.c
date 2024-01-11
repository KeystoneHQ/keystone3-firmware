#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_create_share_widgets.h"

static int32_t GuiCreateShareViewInit(uint8_t entropyMethod)
{
    GuiCreateShareInit(entropyMethod);
    return SUCCESS_CODE;
}

static int32_t GuiCreateShareViewDeInit(void)
{
    GuiCreateShareDeInit();
    return SUCCESS_CODE;
}

int32_t GuiCreateShareViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    int32_t ret = SUCCESS_CODE;
    GUI_ASSERT(g_createShareView.isActive);
    uint8_t entropyMethod;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            entropyMethod = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiCreateShareViewInit(entropyMethod);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiCreateShareViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiCreateShareRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiCreateSharePrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiCreateShareNextTile();
        break;
    case SIG_CREATE_SHARE_VIEW_NEXT_SLICE:
        GuiCreateShareNextSlice();
        break;
    case SIG_CREATE_SHARE_UPDATE_MNEMONIC:
        GuiCreateShareUpdateMnemonic(param, usLen);
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS:
        GuiWriteSeResult(true, 0);
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL:
        if (param != NULL) {
            ret = *(int32_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiWriteSeResult(false, ret);
        break;
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

