#ifdef BTC_ONLY
#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_btc_wallet_profile_widgets.h"

static int32_t GuiBtcWalletProfileViewInit(void)
{
    GuiBtcWalletProfileInit();
    return SUCCESS_CODE;
}

static int32_t GuiBtcWalletProfileViewDeInit(void)
{
    GuiBtcWalletProfileDeInit();
    return SUCCESS_CODE;
}

static int32_t GuiBtcWalletProfileViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint16_t tileIndex = 0;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiBtcWalletProfileViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiBtcWalletProfileViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiBtcWalletProfileRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        if (param != NULL) {
            tileIndex = *(uint8_t *)param;
        } else {
            GuiBtcWalletProfilePrevTile(0);
        }
        break;
    case SIG_SETUP_VIEW_TILE_NEXT:
        if (param != NULL) {
            tileIndex = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiBtcWalletProfileNextTile(tileIndex);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_btcBtcWalletProfileView = {
    .id = SCREEN_BTC_WALLET_PROFILE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiBtcWalletProfileViewEventProcess,
};
#endif
