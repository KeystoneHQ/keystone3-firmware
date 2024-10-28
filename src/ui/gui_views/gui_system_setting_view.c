#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_system_setting_widgets.h"
#include "gui_enter_passcode.h"
#include "gui_lock_widgets.h"
#include "gui_model.h"
#include "gui_enter_passcode.h"

int32_t GuiSystemSettingViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_systemSettingView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiSystemSettingAreaInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiSystemSettingAreaDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiSystemSettingAreaRefresh();
        break;
    case GUI_EVENT_UPDATE_LANGUAGE:
        GuiSystemSettingLanguage(param);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            } else if (sig == SIG_SETTING_CHANGE_PERMIT_SWITCH) {
                GuiDealChangePermitKeyBoard(true);
                return SUCCESS_CODE;
            }
        }
        GuiSystemSettingVerifyPasswordSuccess();
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
        GuiSystemSettingVerifyPasswordErrorCount(param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_systemSettingView = {
    .id = SCREEN_SYSTEM_SETTING,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiSystemSettingViewEventProcess,
};
