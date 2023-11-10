#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_qrcode_widgets.h"
#include "gui_lock_widgets.h"

static int32_t GuiUSBSignViewInit(void)
{
    GuiUSBTransportInit();
    return SUCCESS_CODE;
}

static int32_t GuiQrCodeViewDeInit(void)
{
    GuiQrCodeDeInit();
    return SUCCESS_CODE;
}

int32_t GuiUSBSignViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiUSBSignViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiQrCodeViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiQrCodeRefresh();
        break;
    case SIG_QRCODE_VIEW_SCAN_FAIL:
        GuiQrCodeScanResult(false, param);
        break;
    case SIG_QRCODE_VIEW_SCAN_PASS:
        GuiQrCodeScanResult(true, param);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiQrCodeVerifyPasswordSuccess();
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
        GuiLockScreenPassCode(false);
        GuiQrCodeVerifyPasswordErrorCount(param);
        break;
    case SIG_FINGER_RECOGNIZE_RESPONSE:
        GuiQrCodeDealFingerRecognize(param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_USBSignView = {
    .id = SCREEN_USB_SIGN,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiUSBSignViewEventProcess,
};

