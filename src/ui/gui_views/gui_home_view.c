#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_home_widgets.h"

static int32_t GuiHomeViewInit(void)
{
    GuiHomeAreaInit();
    GuiModeGetWalletDesc();
    return SUCCESS_CODE;
}

static int32_t GuiHomeViewDeInit(void)
{
    GuiHomeDeInit();
    return SUCCESS_CODE;
}

int32_t GuiHomeViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiHomeViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiHomeViewDeInit();
    case GUI_EVENT_DISACTIVE:
        GuiHomeDisActive();
        break;
    case GUI_EVENT_RESTART:
        GuiHomeRestart();
        break;
    case GUI_EVENT_REFRESH:
        GuiHomeRefresh();
        if (param != NULL) {
            GuiModeGetWalletDesc();
        }
        break;
    case SIG_INIT_GET_CURRENT_WALLET_DESC:
        GuiHomeSetWalletDesc((WalletDesc_t *)param);
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL)
        {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *)passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS)
            {
                GuiLockScreenPassCode(false);
                GuiHomePasswordErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiLockScreenPassCode(false);
        GuiHomePasswordErrorCount(param);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        printf("SIG_VERIFY_PASSWORD_PASS\n");
        if (param != NULL)
        {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS)
            {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiRemoveKeyboardWidget();
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD_START:
        GuiPendingHintBoxOpen(_("Pending"), _("generating_qr_codes"));
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD_PASS:
        GuiPendingHintBoxRemove();
        GuiContinueToReceiveArPage();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_homeView = {
    .id = SCREEN_HOME,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiHomeViewEventProcess,
};
