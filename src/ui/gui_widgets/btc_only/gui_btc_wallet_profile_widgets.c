#ifdef BTC_ONLY
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

typedef enum {
    WALLET_PROFILE_SELECT = 0,
    WALLET_PROFILE_MULTI_WALLET,

} WALLET_PROFILE_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
} WalletProfileWidgets_t;

typedef struct {
    lv_obj_t *button;
    lv_obj_t *label;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} Checkbox_t;

static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent);
static void CreateMultiSigWalletWidget(lv_obj_t *parent);
static void CreateBtcNetworkWidget(lv_obj_t *parent);
static void NetworkHandler(lv_event_t *e);
static void ExportXpubHandler(lv_event_t *e);
static void EmptyHandler(lv_event_t *e);
static void AddMultiSigWalletHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_networkCont;
static WalletProfileWidgets_t g_walletProfile;
static lv_obj_t *g_noticeWindow = NULL;
static lv_obj_t *g_networkSwitch = NULL;

void GuiBtcWalletProfileInit(void)
{
    g_pageWidget = CreatePageWidget();
    g_walletProfile.tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(g_walletProfile.tileView, WALLET_PROFILE_SELECT, 0, LV_DIR_HOR);
    CreateBtcWalletProfileEntranceWidget(tile);

    tile = lv_tileview_add_tile(g_walletProfile.tileView, WALLET_PROFILE_MULTI_WALLET, 0, LV_DIR_HOR);
    CreateMultiSigWalletWidget(tile);

    g_walletProfile.currentTile = WALLET_PROFILE_SELECT;

    GuiBtcWalletProfileRefresh();
}


void GuiBtcWalletProfileDeInit(void)
{
    GUI_DEL_OBJ(g_networkCont);
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
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_multi_sign_title"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
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
    lv_obj_t *button = GuiSettingItemButton(parent, 456, _("wallet_profile_single_sign_title"), _("wallet_profile_single_sign_desc"), &imgKey, EmptyHandler, NULL);
    lv_obj_set_height(button, 118);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 0);

    button = GuiSettingItemButton(parent, 456, _("wallet_profile_multi_sign_title"), NULL, &imgTwoKey, NextTileHandler, NULL);
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

    // button = GuiSettingItemButton(parent, 456, _("wallet_profile_export_title"), NULL, &imgExport, ExportXpubHandler, NULL);
    // lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 346);
}

static void CreateMultiSigWalletWidget(lv_obj_t *parent)
{
    lv_obj_t *img = GuiCreateImg(parent, &imgLockMulti);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 128);

    lv_obj_t *label = GuiCreateTextLabel(parent, _("wallet_profile_no_multi_wallet_notice"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 232);

    lv_obj_t *button = GuiCreateBtn(parent, _("wallet_profile_add_multi_wallet"));
    lv_obj_set_width(button, 408);
    lv_obj_set_style_text_color(lv_obj_get_child(button, 0), ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 436);
    lv_obj_add_event_cb(button, AddMultiSigWalletHandler, LV_EVENT_CLICKED, NULL);

    button = GuiCreateBtn(parent, _("wallet_profile_multi_wallet_show_xpub"));
    lv_obj_set_width(button, 408);
    lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 526);
    lv_obj_add_event_cb(button, AddMultiSigWalletHandler, LV_EVENT_CLICKED, NULL);
}

static void ExportXpubHandler(lv_event_t *e)
{
    HOME_WALLET_CARD_ENUM chainCard = HOME_WALLET_CARD_BTC;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenViewWithParam(&g_exportPubkeyView, &chainCard, sizeof(chainCard));
    }
}


static void EmptyHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("do nothing\n");
    }
}

static void OpenCreateMultiViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow)
        GuiFrameOpenView(lv_event_get_user_data(e));
    }
}

static void AddMultiSigWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("wallet_profile_create_multi_wallet"), .src = &imgArrowRight, .callBack = OpenCreateMultiViewHandler, &g_createMultiView},
        {.name = _("wallet_profile_import_multi_wallet"), .src = &imgArrowRight, .callBack = OpenCreateMultiViewHandler, &g_createMultiView},
    };

    if (code == LV_EVENT_CLICKED) {
        g_noticeWindow = GuiCreateMoreInfoHintBox(&imgClose, _("wallet_profile_add_multi_wallet"), moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), false);
        lv_obj_t *closeBtn = GuiCreateImgButton(g_noticeWindow,  &imgClose, 64, CloseHintBoxHandler, &g_noticeWindow);
        GuiAlignToPrevObj(closeBtn, LV_ALIGN_LEFT_MID, 358, 0);
    }
}
#endif
