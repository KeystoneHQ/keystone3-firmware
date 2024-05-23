#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_single_phrase_widgets.h"

int32_t GuiSinglePhraseViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    int32_t ret = SUCCESS_CODE;
    GUI_ASSERT(g_singlePhraseView.isActive);
    uint8_t entropyMethod;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            entropyMethod = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiSinglePhraseInit(entropyMethod);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiSinglePhraseDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiSinglePhraseRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiSinglePhrasePrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiSinglePhraseNextTile();
        break;
    case SIG_CREAT_SINGLE_PHRASE_TON_GENERATION_START:
        GuiShowTonGeneratingModal(true);
        break;
    case SIG_CREAT_SINGLE_PHRASE_TON_GENERATION_END:
        GuiShowTonGeneratingModal(false);
        break;
    case SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC:
        GuiSinglePhraseUpdateMnemonic(param, usLen);
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
    case SIG_SETTING_CHANGE_WALLET_DESC_PASS:
        return SUCCESS_CODE;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_singlePhraseView = {
    .id = SINGLE_PHRASE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSinglePhraseViewEventProcess,
};

