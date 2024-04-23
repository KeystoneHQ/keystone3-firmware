#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_connect_wallet_widgets.h"
#include "gui_pending_hintbox.h"

int32_t GuiConnectWalletViewInit(void)
{
    GuiConnectWalletInit();
    return SUCCESS_CODE;
}

int32_t GuiConnectWalletViewDeInit(void)
{
    GuiConnectWalletDeInit();
    return SUCCESS_CODE;
}

int32_t GuiConnectWalletViewEventProcess(void* self, uint16_t usEvent, void* param, uint16_t usLen)
{
    GUI_ASSERT(g_connectWalletView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiConnectWalletViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiConnectWalletViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiConnectWalletRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiConnectWalletPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiConnectWalletNextTile();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiConnectWalletHandleURGenerate((char*)param, usLen);
        break;
    case SIG_BACKGROUND_UR_UPDATE:
        GuiConnectWalletHandleURUpdate((char*)param, usLen);
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *) passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiConnectWalletPasswordErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiLockScreenPassCode(false);
        GuiConnectWalletPasswordErrorCount(param);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        printf("SIG_VERIFY_PASSWORD_PASS\n");
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiPrepareArConnectWalletView();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_connectWalletView = {
    .id = CONNECT_WALLET,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiConnectWalletViewEventProcess,
};

