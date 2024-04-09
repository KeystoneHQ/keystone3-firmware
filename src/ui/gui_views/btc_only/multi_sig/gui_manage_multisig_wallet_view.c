#include "gui.h"
#include "gui_views.h"
#include "gui_manage_multisig_wallet_widgets.h"
#include "gui_lock_widgets.h"

int32_t GuiManageMultiViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_manageMultisigWalletView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiManageMultisigWalletInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiManageMultisigWalletDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiManageMultisigWalletRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiManageMultiWalletPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiManageMultisigWalletNextTile(0xFF);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        DeleteMultisigWallet();
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param == NULL) {
            return ERR_GUI_ERROR;
        }
        GuiManageMultisigPasswordErrorCount(param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_manageMultisigWalletView = {
    .id = SCREEN_MANAGE_MULTI_SIG,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiManageMultiViewEventProcess,
};

