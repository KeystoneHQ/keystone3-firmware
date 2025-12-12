#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_passphrase_widgets.h"

static int32_t GuiPassphraseViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_passphraseView = {
    .id = SCREEN_PASSPHRASE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiPassphraseViewEventProcess,
};
