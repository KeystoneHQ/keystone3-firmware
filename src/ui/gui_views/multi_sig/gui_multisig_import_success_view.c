#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_multisig_import_wallet_success_widgets.h"

static int32_t GuiImportWalletSuccessViewInit(char* verifyCode)
{
    GuiImportMultisigWalletSuccessWidgetsInit(verifyCode);
    return SUCCESS_CODE;
}

static int32_t GuiImportWalletSuccessViewDeInit(void)
{
    GuiImportMultisigWalletSuccessWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiImportWalletSuccessViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_multisigImportWalletSuccessView.isActive);
    char* verifyCode;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if(param == NULL){
            return ERR_GUI_ERROR;
        }
        verifyCode = (char *)param;
        return GuiImportWalletSuccessViewInit(verifyCode);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiImportWalletSuccessViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiImportMultisigWalletSuccessWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiImportMultisigWalletSuccessWidgetsRestart();
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

GUI_VIEW g_multisigImportWalletSuccessView = {
    .id = SCREEN_MULTI_SIG_IMPORT_WALLET_SUCCESS,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiImportWalletSuccessViewEventProcess,
};
