#include "gui_btc_wallet_profile_widgets.h"
#include "gui.h"
#include "gui_views.h"
#include "gui_model.h"
#include "gui_status_bar.h"
#include "gui_page.h"
#include "gui_button.h"
#include "gui_btc_home_widgets.h"
#include "gui_hintbox.h"
#include "gui_api.h"
#include "multi_sig_wallet_manager.h"

typedef enum {
    WALLET_PROFILE_SELECT = 0,
    WALLET_PROFILE_MULTI_WALLET,

} WALLET_PROFILE_ENUM;

typedef struct {
    lv_obj_t *button;
    lv_obj_t *label;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} Checkbox_t;

static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent);
static void CreateBtcNetworkWidget(lv_obj_t *parent);
static void NetworkHandler(lv_event_t *e);
static void ManageMultiSigWalletHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_networkSwitch = NULL;

void GuiBtcWalletProfileInit(void)
{
    g_pageWidget = CreatePageWidget();
    CreateBtcWalletProfileEntranceWidget(g_pageWidget->contentZone);
}

void GuiBtcWalletProfileDeInit(void)
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiBtcWalletProfileRefresh(void)
{
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_mid_btn"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
}

static void SwitchTestnetHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        bool en = lv_obj_has_state(g_networkSwitch, LV_STATE_CHECKED);
        SetIsTestNet(!en);
        GuiApiEmitSignal(SIG_STATUS_BAR_TEST_NET, NULL, 0);
        if (en) {
            lv_obj_clear_state(g_networkSwitch, LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(g_networkSwitch, LV_STATE_CHECKED);
        }
    }
}

static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t *button = GuiCreateSettingItemButton(parent, 456, _("wallet_profile_single_sign_title"), _("wallet_profile_single_sign_desc"), 
                                                         &imgKey, &imgArrowRight, UnHandler, NULL);
    lv_obj_set_height(button, 118);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 0);

    button = GuiCreateSettingItemButton(parent, 456, _("wallet_profile_multi_sign_title"), NULL, &imgTwoKey, 
                                               &imgArrowRight, OpenViewHandler, &g_manageMultisigWalletView);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 130);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, 226);

    lv_obj_t *label = GuiCreateTextLabel(parent, _("wallet_profile_network_test"));
    g_networkSwitch = GuiCreateSwitch(parent);
    if (GetIsTestNet()) {
        lv_obj_add_state(g_networkSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_networkSwitch, LV_STATE_CHECKED);
    }
    lv_obj_clear_flag(g_networkSwitch, LV_OBJ_FLAG_CLICKABLE);
    GuiButton_t table[] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0}},
        {.obj = g_networkSwitch, .align = LV_ALIGN_LEFT_MID, .position = {376, 0}}
    };
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), SwitchTestnetHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 239);
}