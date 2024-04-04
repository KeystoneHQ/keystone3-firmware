#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_import_multisig_wallet_info_widgets.h"

int32_t GuiImportWalletInfoViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_importMultisigWalletInfoView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiImportMultisigWalletInfoWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiImportMultisigWalletInfoWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiImportMultisigWalletInfoWidgetsRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiImportMultisigWalletInfoWidgetsRestart();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiImportMultiInfoPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiImportMultiInfoNextTile();
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
    .pEvtHandler = GuiImportWalletInfoViewEventProcess,
};
