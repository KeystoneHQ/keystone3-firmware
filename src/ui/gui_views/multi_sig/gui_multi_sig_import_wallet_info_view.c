#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_multi_sig_import_wallet_info_widgets.h"

static int32_t GuiImportWalletInfoViewInit(void)
{
    GuiImportMultisigWalletInfoWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiImportWalletInfoViewDeInit(void)
{
    GuiImportMultisigWalletInfoWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiImportWalletInfoViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_multiSigImportWalletInfoView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiImportWalletInfoViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiImportWalletInfoViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiImportMultisigWalletInfoWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiImportMultisigWalletInfoWidgetsRestart();
        break;
    case SIG_MULTISIG_WALLET_IMPORT_VERIFY_PASSWORD:
        GuiImportMultisigWalletInfoVerifyPasswordSuccess();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_multiSigImportWalletInfoView = {
    .id = SCREEN_MULTI_SIG_IMPORT_WALLET_INFO,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiImportWalletInfoViewEventProcess,
};
