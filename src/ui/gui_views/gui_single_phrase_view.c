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

