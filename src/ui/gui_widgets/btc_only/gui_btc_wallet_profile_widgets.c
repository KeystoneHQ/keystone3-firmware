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
#include "gui_export_pubkey_widgets.h"
#include <stdio.h>

typedef enum {
    WALLET_PROFILE_SELECT = 0,
    WALLET_PROFILE_SINGLE_WALLET,

} WALLET_PROFILE_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
} WalletProfileWidgets_t;

static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent);
static void SetDefaultSingleWalletHandler(lv_event_t *e);
static void CreateSingleSigWalletWidget(lv_obj_t *parent);

static WalletProfileWidgets_t g_walletProfile;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_networkSwitch = NULL;
static lv_obj_t *g_setDefaultBtn = NULL;

void GuiBtcWalletProfileInit(void)
{
    g_pageWidget = CreatePageWidget();
    g_walletProfile.tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(g_walletProfile.tileView, WALLET_PROFILE_SELECT, 0, LV_DIR_HOR);
    CreateBtcWalletProfileEntranceWidget(tile);

    tile = lv_tileview_add_tile(g_walletProfile.tileView, WALLET_PROFILE_SINGLE_WALLET, 0, LV_DIR_HOR);
    CreateSingleSigWalletWidget(tile);

    g_walletProfile.currentTile = WALLET_PROFILE_SELECT;

    GuiBtcWalletProfileRefresh();
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
    if (g_walletProfile.currentTile == WALLET_PROFILE_SELECT) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_mid_btn"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    } else {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_single_sign_title"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        lv_obj_t *label = lv_obj_get_child(g_setDefaultBtn, 0);
        if (GetDefaultWalletIndex() == SINGLE_WALLET) {
            lv_obj_clear_flag(g_setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
            lv_label_set_text(label, _("wallet_profile_current_default_desc"));
        } else {
            lv_obj_add_flag(g_setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
            lv_label_set_text(label, _("manage_multi_wallet_set_default"));
        }
    }
}

int8_t GuiBtcWalletProfilePrevTile(void)
{
    --g_walletProfile.currentTile;
    lv_obj_set_tile_id(g_walletProfile.tileView, g_walletProfile.currentTile, 0, LV_ANIM_OFF);
    GuiBtcWalletProfileRefresh();
    return 0;
}

int8_t GuiBtcWalletProfileNextTile(void)
{
    ++g_walletProfile.currentTile;
    lv_obj_set_tile_id(g_walletProfile.tileView, g_walletProfile.currentTile, 0, LV_ANIM_OFF);
    GuiBtcWalletProfileRefresh();
    return 0;
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
    DEFAULT_WALLET_INDEX_ENUM defaultWallet = GetDefaultWalletIndex();
    char *singleWalletDesc = (char *)_("wallet_profile_default_desc"), *multiWalletDesc = NULL;
    uint16_t singleWalletHeight = 118, multiWalletDescHeight = 84;
    if (defaultWallet != SINGLE_WALLET) {
        singleWalletDesc = NULL;
        singleWalletHeight = 84;
        multiWalletDesc = (char *)_("wallet_profile_default_desc");
        multiWalletDescHeight = 118;
    }
    lv_obj_t *button = GuiCreateSettingItemButton(parent, 456, _("wallet_profile_single_sign_title"), singleWalletDesc, &imgKey,
                       &imgArrowRight, NextTileHandler, NULL);
    lv_obj_set_height(button, singleWalletHeight);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 0);

    button = GuiCreateSettingItemButton(parent, 456, _("wallet_profile_multi_sign_title"), multiWalletDesc, &imgTwoKey,
                                        &imgArrowRight, OpenViewHandler, &g_manageMultisigWalletView);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, singleWalletHeight + 12);
    lv_obj_set_height(button, multiWalletDescHeight);

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

static void CreateSingleSigWalletWidget(lv_obj_t *parent)
{
    static HOME_WALLET_CARD_ENUM chainCard = HOME_WALLET_CARD_BTC;

    lv_obj_t *button = GuiCreateSelectButton(parent, _("manage_multi_wallet_set_default"), &imgDefaultWallet,
                       SetDefaultSingleWalletHandler, NULL, true);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 0);
    g_setDefaultBtn = button;

    button = GuiCreateSelectButton(parent, _("manage_multi_wallet_export_config"), &imgWalletExport,
                                   OpenExportViewHandler, &chainCard, true);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 92);
}

static void SetDefaultSingleWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        SetDefaultWalletIndex((DEFAULT_WALLET_INDEX_ENUM)SINGLE_WALLET);
        lv_obj_t *label = lv_obj_get_child(g_setDefaultBtn, 0);
        lv_obj_clear_flag(g_setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
        lv_label_set_text(label, _("wallet_profile_current_default_desc"));
    }
}