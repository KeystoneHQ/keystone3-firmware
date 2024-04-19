#include "gui.h"
#include "gui_views.h"
#include "gui_button.h"
#include "gui_resource.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_home_widgets.h"
#include "user_memory.h"
#include "gui_hintbox.h"
#include "gui_obj.h"
#include "gui_analyze.h"
#include "gui_chain.h"
#include "account_public_info.h"
#include "gui_keyboard.h"
#include "gui_model.h"
#include "gui_web_auth_widgets.h"
#include "gui_setup_widgets.h"
#include "keystore.h"
#include "gui_page.h"
#include "account_manager.h"
#include "log_print.h"
#include "gui_btc_home_widgets.h"
#include "gui_multisig_read_sdcard_widgets.h"

static lv_obj_t *g_manageWalletLabel = NULL;
static lv_obj_t *g_homeWalletCardCont = NULL;
static lv_obj_t *g_homeViewCont = NULL;
static lv_obj_t *g_manageCont = NULL;
static lv_obj_t *g_moreHintbox = NULL;
static bool g_isManageOpen = false;
static bool g_isManageClick = true;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_twoKeyImg = NULL;
static lv_timer_t *g_countDownTimer = NULL; // count down timer

static WalletState_t g_walletState[] = {
    {HOME_WALLET_CARD_BTC, false, "BTC", true, false, SINGLE_WALLET}
};
static WalletState_t g_walletBakState[HOME_WALLET_CARD_BUTT] = {0};

static const ChainCoinCard_t g_coinCardArray[HOME_WALLET_CARD_BUTT] = {
    {
        .index = HOME_WALLET_CARD_BTC,
        .coin = "BTC",
        .chain = "Bitcoin",
        .icon = &coinBtc,
    },
};

void ScanQrCodeHandler(lv_event_t *e);
static void OpenWalletProfileHandler(lv_event_t *e);
static void CreateHomePageButtons(void);
static void RcvHandler(lv_event_t *e);
static void AddFlagCountDownTimerHandler(lv_timer_t *timer);
void AccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count);

static void UpdateManageWalletState(bool needUpdate)
{
    char tempBuf[BUFFER_SIZE_16] = {0};
    uint8_t selectCnt = 0;
    g_isManageOpen = false;
    int total = 0;
    for (int i = 0; i < HOME_WALLET_CARD_BUTT; i++) {

        if (g_walletState[i].enable) {
            total++;
        }
        if (g_walletBakState[i].state == true) {
            selectCnt++;
            lv_obj_add_state(g_walletState[i].checkBox, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(g_walletState[i].checkBox, LV_STATE_CHECKED);
        }
    }
    snprintf_s(tempBuf, BUFFER_SIZE_16, _("home_select_coin_count_fmt"), selectCnt, total);
    lv_label_set_text(g_manageWalletLabel, tempBuf);
    if (needUpdate) {
        if (memcmp(g_walletState, g_walletBakState, sizeof(g_walletState))) {
            memcpy(g_walletState, g_walletBakState, sizeof(g_walletBakState));
            AccountPublicHomeCoinSet(g_walletState, NUMBER_OF_ARRAYS(g_walletState));
        }
    }
}

bool GuiHomePageIsTop(void)
{
    return GuiCheckIfTopView(&g_homeView) && g_manageCont == NULL;
}

void ReturnManageWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        UpdateManageWalletState(false);
        GUI_DEL_OBJ(g_manageCont);
        GuiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
    }
}

static void CreateHomePageButtons(void)
{
    lv_obj_t *img, *label, *arrow, *rcvButton, *scanButton;

    img = GuiCreateImg(g_homeWalletCardCont, &imgReceive);
    label = GuiCreateLittleTitleLabel(g_homeWalletCardCont, _("home_button_receive"));
    arrow = GuiCreateImg(g_homeWalletCardCont, &imgArrowRight);
    g_twoKeyImg = arrow;
    GuiButton_t rcvButtonTable[3] = {
        {
            .obj = img,
            .align = LV_ALIGN_TOP_LEFT,
            .position = {36, 36},
        },
        {
            .obj = label,
            .align = LV_ALIGN_TOP_LEFT,
            .position = {36, 108},
        },
        {
            .obj = arrow,
            .align = LV_ALIGN_TOP_RIGHT,
            .position = {-24, 110},
        },
    };
    rcvButton = GuiCreateButton(g_homeWalletCardCont, 432, 172, rcvButtonTable, NUMBER_OF_ARRAYS(rcvButtonTable), RcvHandler, NULL);
    lv_obj_align(rcvButton, LV_ALIGN_BOTTOM_MID, 0, -220);
    lv_obj_set_style_bg_color(rcvButton, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rcvButton, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rcvButton, WHITE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rcvButton, LV_OPA_20, LV_STATE_PRESSED | LV_PART_MAIN);

    img = GuiCreateImg(g_homeWalletCardCont, &imgScan48);
    label = GuiCreateLittleTitleLabel(g_homeWalletCardCont, _("home_button_scan"));
    arrow = GuiCreateImg(g_homeWalletCardCont, &imgArrowRight);
    GuiButton_t scanButtonTable[3] = {
        {
            .obj = img,
            .align = LV_ALIGN_TOP_LEFT,
            .position = {36, 36},
        },
        {
            .obj = label,
            .align = LV_ALIGN_TOP_LEFT,
            .position = {36, 108},
        },
        {
            .obj = arrow,
            .align = LV_ALIGN_TOP_RIGHT,
            .position = {-24, 110},
        },
    };
    scanButton = GuiCreateButton(g_homeWalletCardCont, 432, 172, scanButtonTable, NUMBER_OF_ARRAYS(scanButtonTable), ScanQrCodeHandler, NULL);
    lv_obj_align(scanButton, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_style_bg_color(scanButton, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scanButton, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(scanButton, ORANGE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scanButton, LV_OPA_80, LV_STATE_PRESSED | LV_PART_MAIN);
}

static void RcvHandler(lv_event_t *e)
{
    static HOME_WALLET_CARD_ENUM coin;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("rcv handler\n");
        coin = HOME_WALLET_CARD_BTC;
        ShowWallPaper(false);
        GuiFrameOpenViewWithParam(&g_utxoReceiveView, &coin, sizeof(coin));
    }
}

void ScanQrCodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_isManageClick = false;
        if (g_countDownTimer != NULL) {
            lv_timer_del(g_countDownTimer);
            g_countDownTimer = NULL;
        }
        ShowWallPaper(false);
        GuiFrameOpenView(&g_scanView);
    }
}

void ConfirmManageAssetsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        UpdateManageWalletState(true);
        GUI_DEL_OBJ(g_manageCont)
        GuiHomeRefresh();
    }
}

static void OpenMoreViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        g_moreHintbox = NULL;
        ShowWallPaper(false);
        GuiFrameOpenView(lv_event_get_user_data(e));
    }
}

static void GuiOpenSignBySDCardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ShowWallPaper(false);
        if (SdCardInsert()) {
            static uint8_t fileFilterType = ONLY_PSBT;
            GuiFrameOpenViewWithParam(&g_multisigReadSdcardView, &fileFilterType, sizeof(fileFilterType));
        } else {
            g_moreHintbox = GuiCreateErrorCodeWindow(ERR_UPDATE_SDCARD_NOT_DETECTED, &g_moreHintbox, NULL);
        }
    }
}

static void GuiMoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        MoreInfoTable_t moreInfoTable[] = {
            {.name = _("home_more_connect_wallet"), .src = &imgConnect, .callBack = OpenMoreViewHandler, &g_connectWalletView},
            {.name = _("home_more_sign_by_sdcard"), .src = &imgSdCardColor, .callBack = GuiOpenSignBySDCardHandler, NULL},
            {.name = _("home_more_device_setting"), .src = &imgSettings, .callBack = OpenMoreViewHandler, &g_settingView},
        };
        if (code == LV_EVENT_CLICKED) {
            g_moreHintbox = GuiCreateMoreInfoHintBox(NULL, NULL, moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), true, &g_moreHintbox);
        }
    } else {
        MoreInfoTable_t moreInfoTable[] = {
            {.name = _("home_more_connect_wallet"), .src = &imgConnect, .callBack = OpenMoreViewHandler, &g_connectWalletView},
            {.name = _("home_more_device_setting"), .src = &imgSettings, .callBack = OpenMoreViewHandler, &g_settingView},
        };
        if (code == LV_EVENT_CLICKED) {
            g_moreHintbox = GuiCreateMoreInfoHintBox(NULL, NULL, moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), true, &g_moreHintbox);
        }
    }
}

static void OpenMoreSettingHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_moreHintbox = GuiCreateHintBox(228, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_moreHintbox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_moreHintbox);
        lv_obj_t *label = GuiCreateTextLabel(g_moreHintbox, _("home_more_connect_wallet"));
        lv_obj_t *img = GuiCreateImg(g_moreHintbox, &imgConnect);
        GuiButton_t table[2] = {
            {
                .obj = img,
                .align = LV_ALIGN_LEFT_MID,
                .position = {24, 0},
            },
            {
                .obj = label,
                .align = LV_ALIGN_LEFT_MID,
                .position = {76, 0},
            },
        };
        lv_obj_t *btn = GuiCreateButton(g_moreHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                        OpenMoreViewHandler, &g_connectWalletView);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 596);

        label = GuiCreateTextLabel(g_moreHintbox, _("home_more_device_setting"));
        img = GuiCreateImg(g_moreHintbox, &imgSettings);
        table[0].obj = img;
        table[1].obj = label;

        btn = GuiCreateButton(g_moreHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                              OpenMoreViewHandler, &g_settingView);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 692);
    }
}

static void OpenWalletProfileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        printf("OpenWalletProfileHandler\n");
        ShowWallPaper(false);
        GuiFrameOpenView(&g_btcBtcWalletProfileView);
    }
}

void GuiHomeSetWalletDesc(WalletDesc_t *wallet)
{
    GuiNvsBarSetWalletName((const char *)wallet->name);
    GuiSetEmojiIconIndex(wallet->iconIndex);
    SetStatusBarEmojiIndex(wallet->iconIndex);
    GuiNvsBarSetWalletIcon(GuiGetEmojiIconImg());
}

void GuiHomeAreaInit(void)
{
    g_pageWidget = CreatePageWidget();
    g_homeViewCont = g_pageWidget->contentZone;
#if (WALLPAPER_ENABLE == 1)
    lv_obj_set_style_bg_opa(g_pageWidget->contentZone, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_pageWidget->page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_pageWidget->navBar, LV_OPA_TRANSP, LV_PART_MAIN);
#endif

    lv_obj_t *walletCardCont = GuiCreateContainerWithParent(g_homeViewCont, lv_obj_get_width(lv_scr_act()),
                               lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(walletCardCont, LV_ALIGN_DEFAULT);
    lv_obj_add_flag(walletCardCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(walletCardCont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(walletCardCont, LV_OPA_TRANSP, LV_PART_MAIN);
    g_homeWalletCardCont = walletCardCont;
    CreateHomePageButtons();
    ShowWallPaper(true);
}

void GuiHomeDisActive(void)
{
    if (g_homeWalletCardCont != NULL) {
        lv_obj_add_flag(g_homeWalletCardCont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void AddFlagCountDownTimerHandler(lv_timer_t *timer)
{
    g_isManageClick = true;
    lv_timer_del(timer);
    g_countDownTimer = NULL;
    UNUSED(g_countDownTimer);
}

void GuiHomeRestart(void)
{
    GUI_DEL_OBJ(g_manageCont)
    GuiHomeRefresh();
}

void GuiHomeRefresh(void)
{
#ifdef RUST_MEMORY_DEBUG
    PrintRustMemoryStatus();
#endif
    if (GetCurrentAccountIndex() > 2) {
        return;
    }
    printf("GuiHomeRefresh\n");
    ShowWallPaper(true);
    g_countDownTimer = lv_timer_create(AddFlagCountDownTimerHandler, 500, NULL);
    GuiSetSetupPhase(SETUP_PAHSE_DONE);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_MANAGE, OpenWalletProfileHandler, NULL);
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, GuiMoreHandler, NULL);
    if (g_homeWalletCardCont != NULL) {
        lv_obj_clear_flag(g_homeWalletCardCont, LV_OBJ_FLAG_HIDDEN);
    }
    GUI_DEL_OBJ(g_moreHintbox)
    AccountPublicHomeCoinGet(g_walletState, NUMBER_OF_ARRAYS(g_walletState));
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        lv_img_set_src(g_twoKeyImg, &imgTwoKey);
    } else {
        lv_img_set_src(g_twoKeyImg, &imgArrowRight);
    }
}

bool GetIsTestNet(void)
{
    return g_walletState[HOME_WALLET_CARD_BTC].testNet;
}

void SetIsTestNet(bool testNet)
{
    printf("testNet=%d\n", testNet);
    g_walletState[HOME_WALLET_CARD_BTC].testNet = testNet;
    AccountPublicHomeCoinSet(g_walletState, NUMBER_OF_ARRAYS(g_walletState));
}

CURRENT_WALLET_INDEX_ENUM GetCurrentWalletIndex(void)
{
    return g_walletState[HOME_WALLET_CARD_BTC].defaultWallet;
}

void SetCurrentWalletIndex(CURRENT_WALLET_INDEX_ENUM walletIndex)
{
    printf("walletIndex = %d\n", walletIndex);
    g_walletState[HOME_WALLET_CARD_BTC].defaultWallet = walletIndex;
    AccountPublicHomeCoinSet(g_walletState, NUMBER_OF_ARRAYS(g_walletState));
}

const ChainCoinCard_t *GetCoinCardByIndex(HOME_WALLET_CARD_ENUM index)
{
    for (int i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        if (g_coinCardArray[i].index == index) {
            return &g_coinCardArray[i];
        }
    }
    return NULL;
}

void GuiHomeDeInit(void)
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}
