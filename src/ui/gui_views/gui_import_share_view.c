#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_import_share_widgets.h"

static int32_t GuiImportShareViewInit(uint8_t wordsCnt)
{
    GuiImportShareInit(wordsCnt);
    return SUCCESS_CODE;
}

static int32_t GuiImportShareViewDeInit(void)
{
    GuiImportShareDeInit();
    return SUCCESS_CODE;
}

int32_t GuiImportShareViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t wordsCnt = 0;
    int32_t ret = SUCCESS_CODE;
    GUI_ASSERT(g_importShareView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            wordsCnt = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiImportShareViewInit(wordsCnt);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiImportShareViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiImportShareRefresh();
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS:
        GuiImportShareWriteSe(true, 0);
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL:
        if (param != NULL) {
            ret = *(int32_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiImportShareWriteSe(false, ret);
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiImportSharePrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiImportShareNextTile();
        break;
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

