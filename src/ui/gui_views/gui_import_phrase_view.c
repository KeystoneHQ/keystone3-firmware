#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_import_phrase_widgets.h"

int32_t GuiImportPhraseViewInit(uint8_t num)
{
    GuiImportPhraseInit(num);
    return SUCCESS_CODE;
}

int32_t GuiImportPhraseViewDeInit(void)
{
    GuiImportPhraseDeInit();
    return SUCCESS_CODE;
}

int32_t GuiImportPhraseViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t wordcnt;
    int32_t ret;
    GUI_ASSERT(g_importPhraseView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            wordcnt = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiImportPhraseViewInit(wordcnt);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiImportPhraseViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiImportPhraseRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiImportPhrasePrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiImportPhraseNextTile();
        break;
    case SIG_SETUP_SHOW_TON_MNEMONIC_HINT:
        GuiShowTonMnemonicHint();
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS:
        GuiImportPhraseWriteSe(true, 0);
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL:
        if (param != NULL) {
            ret = *(int32_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiImportPhraseWriteSe(false, ret);
        break;
    case GUI_EVENT_UPDATE_KEYBOARD:
        GuiImportPhraseUpdateKeyboard();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_importPhraseView = {
    .id = IMPORT_SINGLE_PHRASE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiImportPhraseViewEventProcess,
};

