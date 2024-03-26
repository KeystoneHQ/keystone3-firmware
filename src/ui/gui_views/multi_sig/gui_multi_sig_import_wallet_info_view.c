#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_multi_sig_import_wallet_info_widgets.h"

static int32_t GuiImportWalletInfoViewInit(void)
{
    GuiImportWalletInfoWidgetsInit();
    return SUCCESS_CODE;
}

static int32_t GuiImportWalletInfoViewDeInit(void)
{
    GuiImportWalletInfoWidgetsDeInit();
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
        GuiImportWalletInfoWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiImportWalletInfoWidgetsRestart();
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
