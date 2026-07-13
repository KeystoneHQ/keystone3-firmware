#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_derive_context_hash_request_widgets.h"
#include "gui_keyboard_hintbox.h"
#include "gui_lock_widgets.h"

int32_t GuiDeriveContextHashRequestViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    bool isUsb = false;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            isUsb = true;
        }
        GuiDeriveContextHashRequestInit(isUsb);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiDeriveContextHashRequestDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiDeriveContextHashRequestRefresh();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiDeriveContextHashWidgetHandleURGenerate((char *)param, usLen);
        break;
    case SIG_BACKGROUND_UR_UPDATE:
        GuiDeriveContextHashWidgetHandleURUpdate((char *)param, usLen);
        break;
    case SIG_BACKGROUND_UR_GENERATE_FAIL:
        GuiDeriveContextHashWidgetHandleURGenerateFail((char *)param);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        DeriveContextHashHiddenKeyboardAndShowAnimateQR();
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *) passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiLockScreenErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiDeriveContextHashPasswordErrorCount(param);
        break;
    case SIG_INIT_PULLOUT_USB:
        GuiDeriveContextHashUsbPullout();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_deriveContextHashRequestView = {
    .id = SCREEN_DERIVE_CONTEXT_HASH_REQUEST,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiDeriveContextHashRequestViewEventProcess,
};
