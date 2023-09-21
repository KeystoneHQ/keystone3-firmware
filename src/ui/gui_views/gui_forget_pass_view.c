#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_forget_pass_widgets.h"
#include "gui_hintbox.h"
#include "gui_lock_widgets.h"

static int32_t GuiForgetViewInit(void *param)
{
    GuiForgetPassInit(param);
    return SUCCESS_CODE;
}

static int32_t GuiForgetViewDeInit(void)
{
    GuiForgetPassDeInit();
    return SUCCESS_CODE;
}

int32_t GuiForgetViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint16_t tileIndex = 0;
    int32_t ret = SUCCESS_CODE;
    // GUI_ASSERT(g_settingView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiForgetViewInit(param);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiForgetViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiForgetPassRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        if (param != NULL) {
            tileIndex = *(uint8_t *)param;
            GuiForgetPassPrevTile(tileIndex);
        } else {
            GuiForgetPassPrevTile(0);
        }
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        if (param != NULL) {
            tileIndex = *(uint8_t *)param;
            GuiForgetPassNextTile(tileIndex);
        } else {
            GuiForgetPassNextTile(0);
        }
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            tileIndex = *(uint16_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        // GuiForgetPassCode(true, tileIndex);
        GuiLockScreenPassCode(true);
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            tileIndex = *(uint16_t *)passwordVerifyResult->signal;
        } else {
            return ERR_GUI_ERROR;
        }
        // GuiForgetPassCode(false, tileIndex);
        GuiLockScreenPassCode(false);
        GuiLockScreenErrorCount(param);
        break;
    case SIG_SETTING_SET_PIN:
        GuiForgetPassSetPinPass((const char *)param);
        break;
    case SIG_SETTING_REPEAT_PIN:
        GuiForgetPassRepeatPinPass((const char *)param);
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS:
        GuiForgetPassResetPass(true, 0);
        break;
    case SIG_FORGET_PASSWORD_SUCCESS:
        GuiForgetPassVerifyResult(true, 0);
        break;
    case SIG_FORGET_PASSWORD_FAIL:
        if (param != NULL) {
            ret = *(int32_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiForgetPassVerifyResult(false, ret);
        break;
    case GUI_EVENT_UPDATE_KEYBOARD:
        GuiForgetPassUpdateKeyboard();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_forgetPassView = {
    .id = SCREEN_FORGET_PASSCODE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiForgetViewEventProcess,
};

