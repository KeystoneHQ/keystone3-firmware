#include "stdio.h"
#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_multisig_wallet_export_widgets.h"

int32_t GuiImportMultiSigWalletViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_multisigWalletExportView.isActive);
    char* verifyCode;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param == NULL) {
            return ERR_GUI_ERROR;
        }
        verifyCode = (char *)param;
        GuiMultisigWalletExportWidgetsInit(verifyCode, usLen);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiMultisigWalletExportWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiMultisigWalletExportWidgetsRefresh();
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

GUI_VIEW g_multisigWalletExportView = {
    .id = SCREEN_MULTISIG_WALLET_EXPORT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiImportMultiSigWalletViewEventProcess,
};
