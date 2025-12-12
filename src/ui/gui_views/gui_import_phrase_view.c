#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_import_phrase_widgets.h"

int32_t GuiImportPhraseViewInit(uint8_t num)
{
    return SUCCESS_CODE;
}

int32_t GuiImportPhraseViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiImportPhraseViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t wordcnt;
    int32_t ret;
    GUI_ASSERT(g_importPhraseView.isActive);

    switch (usEvent) {
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

