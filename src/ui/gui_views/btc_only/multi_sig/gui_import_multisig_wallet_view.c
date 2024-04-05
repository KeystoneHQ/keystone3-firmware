#include "stdio.h"
#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_import_multisig_wallet_widgets.h"

static int32_t GuiImportWalletSuccessViewDeInit(void)
{
    GuiImportMultisigWalletWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiImportMultiSigWalletViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_importMultisigWalletView.isActive);
    char* walletConifg;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param == NULL) {
            return ERR_GUI_ERROR;
        }
        walletConifg = (char *)param;
        GuiImportMultisigWalletWidgetsInit(walletConifg);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiImportMultisigWalletWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiImportMultisigWalletWidgetsRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiImportMultiPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiImportMultiNextTile();
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        GuiImportMultiNextTile();
        GuiImportMultisigWalletInfoVerifyPasswordSuccess();
        break;
    case SIG_BACKGROUND_UR_GENERATE_SUCCESS:
        GuiAnimantingQRCodeFirstUpdate((char*)param, usLen);
        break;
    case SIG_BACKGROUND_UR_UPDATE:
        GuiAnimatingQRCodeUpdate((char*)param, usLen);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_importMultisigWalletView = {
    .id = SCREEN_MULTI_SIG_IMPORT_WALLET_SUCCESS,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiImportMultiSigWalletViewEventProcess,
};
