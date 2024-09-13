#ifndef BTC_ONLY
#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_key_derivation_request_widgets.h"
#include "gui_keyboard_hintbox.h"

int32_t GuiKeyDerivationRequestViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    bool isUsb = false;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            isUsb = true;
        }
        GuiKeyDerivationRequestInit(isUsb);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiKeyDerivationRequestDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiKeyDerivationRequestRefresh();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiKeyDerivationWidgetHandleURGenerate((char*)param, usLen);
        break;
    case SIG_BACKGROUND_UR_UPDATE:
        GuiKeyDerivationWidgetHandleURUpdate((char*)param, usLen);
        break;
    case SIG_USB_HARDWARE_CALL_PARSE_UR:
        UpdateAndParseHardwareCall();
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        HiddenKeyboardAndShowAnimateQR();
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
        GuiKeyDerivePasswordErrorCount(param);
        break;
    case SIG_INIT_PULLOUT_USB:
        GuiKeyDeriveUsbPullout();
        break;
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
#endif
