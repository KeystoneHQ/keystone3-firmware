#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_firmware_update_widgets.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_lock_widgets.h"
#include "gui_prepare_key_pair_widgets.h"

int32_t GuiPrepareKeyPairViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint32_t rcvValue;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiPrepareKeyPairInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiPrepareKeyPairDeInit();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    case SIG_INIT_SDCARD_CHANGE:
        rcvValue = *(uint32_t *)param;
        GuiStatusBarSetSdCard(!rcvValue, false);
    case GUI_EVENT_REFRESH:
        GuiPrepareKeyPairRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiPrepareKeyPairPrevTile();
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        GuiPrepareKeyPairNextTile();
        break;
    case SIG_SET_PUBKEY_VERIFY_SUCCESS:
        GuiPrepareKeyPairOnPubkeyVerifySuccess(param, usLen);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_prepareKeyPairView = {
    .id = SCREEN_PREPARE_KEY_PAIR,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiPrepareKeyPairViewEventProcess,
};

