#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_lock_widgets.h"
#include "keystore.h"
#include "fingerprint_process.h"
#include "gui_firmware_update_widgets.h"
#include "gui_qr_code.h"
#include "gui_animating_qrcode.h"
#include "gui_status_bar.h"
#include "account_manager.h"
#include "gui_lock_device_widgets.h"
#include "gui_api.h"
#include "usb_task.h"
#include "device_setting.h"
#include "drv_aw32001.h"
void GuiLockViewRefreshLanguage(void);
int32_t GuiLockViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    if (g_lockView.isActive) {
        if (usEvent == SIG_LOCK_VIEW_SCREEN_ON_VERIFY) {
            // close forgetPassView if present, ignore general error;
            GuiFrameCLoseView(&g_forgetPassView);
            // close passphraseView if present, ignore general error;
            GuiFrameCLoseView(&g_passphraseView);
            GuiLockScreenTurnOn(param);
            QRCodeControl(true);
            GuiAnimatingQRCodeControl(true);
            return SUCCESS_CODE;
        }
    }
    uint32_t rcvValue;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiLockScreenInit(param);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        break;
    case SIG_INIT_SDCARD_CHANGE:
        rcvValue = *(uint32_t *)param;
        GuiStatusBarSetSdCard(!rcvValue);
        break;
    case GUI_EVENT_REFRESH:
        GuilockScreenRefresh();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    case SIG_VERIFY_FINGER_PASS:
        if (GuiLockScreenIsFirstUnlock() || g_lockDeviceView.isActive) {
            break;
        }
        SetCurrentAccountIndex();
        GuiFpRecognizeResult(true);
    case SIG_LOCK_VIEW_SCREEN_ON_PASSPHRASE_PASS:
    case SIG_VERIFY_PASSWORD_PASS:
        GuiLockScreenClearFirstUnlock();
        GuiLockScreenPassCode(true);
        QRCodeControl(false);
#if (USB_POP_WINDOW_ENABLE == 1)
        if (*(uint16_t *)param == SIG_LOCK_VIEW_VERIFY_PIN) {
            if (GetUSBSwitch() == true && GetUsbDetectState()) {
                GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 1);
            }
        }
#endif
        break;
    case SIG_VERIFY_FINGER_FAIL:
        if (GuiLockScreenIsFirstUnlock() || g_lockDeviceView.isActive) {
            break;
        }
        GuiFpRecognizeResult(false);
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        GuiLockScreenPassCode(false);
        GuiLockScreenErrorCount(param);
        GuiFirmwareUpdateVerifyPasswordErrorCount(param);
        break;
    case SIG_LOCK_VIEW_SCREEN_OFF:
        GuiLockScreenTurnOff();
        break;
    case SIG_LOCK_VIEW_VERIFY_PIN:
        break;
    case SIG_LOCK_VIEW_SCREEN_GO_LOCK_DEVICE_PASS:
        GuiJumpToLockDevicePage();
        break;
    case SIG_FINGER_RECOGNIZE_RESPONSE:
        break;
    case SIG_PASSCODE_SWITCH_TO_PIN:
        GuiLockScreenPasscodeSwitch(true);
        break;
    case SIG_PASSCODE_SWITCH_TO_PASSWORD:
        GuiLockScreenPasscodeSwitch(false);
        break;
    case SIG_EXTENDED_PUBLIC_KEY_NOT_MATCH:
        GuiLockScreenWipeDevice();
        break;
    case SIG_START_GENERATE_XPUB:
        GuiShowGenerateXPubLoading();
        break;
    case SIG_END_GENERATE_XPUB:
        GuiHideGenerateXPubLoading();
        break;
    case GUI_EVENT_CHANGE_LANGUAGE:
        GuiLockViewRefreshLanguage();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_lockView = {
    .id = SCREEN_LOCK,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiLockViewEventProcess,
};

