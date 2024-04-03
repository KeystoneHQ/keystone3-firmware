// #ifdef BTC_ONLY
#if 1
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
static void NetworkSelHandler(lv_event_t *e);
static void NetworkHandler(lv_event_t *e);
static void ExportXpubHandler(lv_event_t *e);
static void EmptyHandler(lv_event_t *e);
static void AddMultiSigWalletHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_networkCont;
static WalletProfileWidgets_t g_walletProfile;
static lv_obj_t *g_noticeWindow = NULL;
Checkbox_t g_networkCheckbox[2];

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
        // SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, MoreHandler, NULL);
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


static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t *button = GuiSettingItemButton(parent, 456, _("wallet_profile_single_sign_title"), _("wallet_profile_single_sign_desc"), &imgKey, EmptyHandler, NULL);
    lv_obj_set_height(button, 118);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 0);

    button = GuiSettingItemButton(parent, 456, _("wallet_profile_multi_sign_title"), NULL, &imgTwoKey, NextTileHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 130);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, 226);

    button = GuiSettingItemButton(parent, 456, _("wallet_profile_network_title"), NULL, &imgNetwork, NetworkHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 246);
    button = GuiSettingItemButton(parent, 456, _("wallet_profile_export_title"), NULL, &imgExport, ExportXpubHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 346);
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

static void CreateBtcNetworkWidget(lv_obj_t *parent)
{
    lv_obj_t *label, *closeBtn, *closeImg;
    g_networkCont = GuiCreateHintBox(parent, 480, 282, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_networkCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_networkCont);
    label = GuiCreateIllustrateLabel(g_networkCont, "Network");
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -222);

    closeBtn = GuiCreateContainerWithParent(g_networkCont, 64, 64);
    lv_obj_align(closeBtn, LV_ALIGN_BOTTOM_RIGHT, -24, -205);
    lv_obj_set_style_bg_opa(closeBtn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_flag(closeBtn, LV_OBJ_FLAG_CLICKABLE);
    closeImg = GuiCreateImg(closeBtn, &imgClose);
    lv_obj_align(closeImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(closeBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_networkCont);

    char *netType[] = {"MainNet", "TestNet"};
    for (uint32_t i = 0; i < 2; i++) {
        g_networkCheckbox[i].button = GuiCreateContainerWithParent(g_networkCont, 456, 84);
        lv_obj_align(g_networkCheckbox[i].button, LV_ALIGN_TOP_MID, 0, 608 + i * 96);
        lv_obj_set_style_bg_opa(g_networkCheckbox[i].button, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_add_flag(g_networkCheckbox[i].button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_networkCheckbox[i].button, NetworkSelHandler, LV_EVENT_CLICKED, NULL);

        g_networkCheckbox[i].label = GuiCreateTextLabel(g_networkCheckbox[i].button, netType[i]);
        lv_obj_align(g_networkCheckbox[i].label, LV_ALIGN_LEFT_MID, 24, 0);

        g_networkCheckbox[i].checkedImg = GuiCreateImg(g_networkCheckbox[i].button, &imgMessageSelect);
        lv_obj_align(g_networkCheckbox[i].checkedImg, LV_ALIGN_RIGHT_MID, -28, 0);
        lv_obj_add_flag(g_networkCheckbox[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_networkCheckbox[i].uncheckedImg = GuiCreateImg(g_networkCheckbox[i].button, &imgUncheckCircle);
        lv_obj_align(g_networkCheckbox[i].uncheckedImg, LV_ALIGN_RIGHT_MID, -24, 0);
        lv_obj_clear_flag(g_networkCheckbox[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    if (GetIsTestNet()) {
        lv_obj_add_flag(g_networkCheckbox[0].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_networkCheckbox[0].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_networkCheckbox[1].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_networkCheckbox[1].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(g_networkCheckbox[0].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_networkCheckbox[0].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_networkCheckbox[1].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_networkCheckbox[1].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
}


static void NetworkSelHandler(lv_event_t *e)
{
    uint32_t i, j;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        printf("net work sel\n");
        for (i = 0; i < 2; i++) {
            if (target == g_networkCheckbox[i].button) {
                printf("target is %d\n", i);
                SetIsTestNet(i == 1);
                GuiApiEmitSignal(SIG_STATUS_BAR_TEST_NET, NULL, 0);
                for (j = 0; j < 2; j++) {
                    if (i == j) {
                        //checked
                        lv_obj_add_flag(g_networkCheckbox[j].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_clear_flag(g_networkCheckbox[j].checkedImg, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        //unchecked
                        lv_obj_add_flag(g_networkCheckbox[j].checkedImg, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_clear_flag(g_networkCheckbox[j].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }
        }
        GUI_DEL_OBJ(g_networkCont);
    }
}


static void NetworkHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        CreateBtcNetworkWidget(g_pageWidget->contentZone);
    }
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
