#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_lock_widgets.h"
#include "gui_import_multisig_wallet_info_widgets.h"

int32_t GuiImportMultisigWalletInfoViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_importMultisigWalletInfoView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiImportMultisigWalletInfoWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiImportMultisigWalletInfoWidgetsDeInit();
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiImportMultisigWalletInfoVerifyPasswordSuccess();
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param == NULL) {
            return ERR_GUI_ERROR;
        }
        GuiImportMultisigPasswordErrorCount(param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_importMultisigWalletInfoView = {
    .id = SCREEN_MULTI_SIG_IMPORT_WALLET_INFO,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiImportMultisigWalletInfoViewEventProcess,
};
