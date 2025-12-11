#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_key_derivation_request_widgets.h"
#include "gui_keyboard_hintbox.h"
#include "gui_lock_widgets.h"

int32_t GuiKeyDerivationRequestViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    bool isUsb = false;
    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_keyDerivationRequestView = {
    .id = SCREEN_KEY_DERIVATION_REQUEST,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiKeyDerivationRequestViewEventProcess,
};