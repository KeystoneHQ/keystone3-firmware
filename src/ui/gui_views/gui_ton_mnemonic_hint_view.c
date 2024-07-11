#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_ton_mnemonic_hint_widgets.h"

static int32_t GuiTonMnemonicHintViewInit()
{
    GuiTonMnemonicHintWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiTonMnemonicHintViewDeInit(void)
{
    GuiTonMnemonicHintWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiTonMnemonicHintViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t viewType = 0;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiTonMnemonicHintViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiTonMnemonicHintViewDeInit();
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_tonMnemonicHintView = {
    .id = SCREEN_TON_MNEMONIC_HINT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiTonMnemonicHintViewEventProcess,
};