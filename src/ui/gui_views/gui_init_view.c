#include "gui_obj.h"
#include "gui_views.h"
#include "gui_style.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_enter_passcode.h"
#include "gui_lock_widgets.h"
#include "presetting.h"
#include "anti_tamper.h"

static int32_t GuiInitViewInit(void)
{
    LanguageInit();
    GuiEnterPassLabelInit();
    GuiStyleInit();
    // if (GetFactoryResult() == false) {
    //     GuiFrameOpenView(&g_inactiveView);
    //     return SUCCESS_CODE;
    // }
    // if (Tampered()) {
    //     GuiFrameOpenView(&g_selfDestructView);
    //     return SUCCESS_CODE;
    // }
    GuiModeGetAmount();
    // GuiFrameOpenView(&g_settingView);
    // GuiFrameOpenView(&g_connectWalletView);
    return SUCCESS_CODE;
}

int32_t GUI_InitViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t walletNum;
    static uint16_t lockParam = SIG_LOCK_VIEW_VERIFY_PIN;
    uint16_t battState;
    uint32_t rcvValue;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiInitViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        printf("init view should not be closed");
        break;
    case SIG_INIT_GET_ACCOUNT_NUMBER:
        walletNum = *(uint8_t *)param;
        if (walletNum == 0) {
            // return GuiFrameOpenView(&g_setupView);
        } else {
            return GuiFrameOpenViewWithParam(&g_lockView, &lockParam, sizeof(lockParam));
        }
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_initView = {
    .id = SCREEN_INIT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GUI_InitViewEventProcess,
};

