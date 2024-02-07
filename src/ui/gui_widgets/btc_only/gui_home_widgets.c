#ifdef BTC_ONLY
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

// static uint8_t g_manageWalletNum = 2;
static lv_obj_t *g_manageWalletLabel = NULL;
static lv_obj_t *g_homeWalletCardCont = NULL;
static lv_obj_t *g_homeViewCont = NULL;
//static lv_obj_t *g_scanImg = NULL;
static lv_obj_t *g_manageCont = NULL;
static lv_obj_t *g_moreHintbox = NULL;
static bool g_isManageOpen = false;
static bool g_isManageClick = true;
static PageWidget_t *g_pageWidget;
static lv_timer_t *g_countDownTimer = NULL; // count down timer
static lv_obj_t *g_walletButton[HOME_WALLET_CARD_BUTT];

static WalletState_t g_walletState[HOME_WALLET_CARD_BUTT] = {
    {HOME_WALLET_CARD_BTC, false, "BTC", true},
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
static void CreateHomePageButtons(void);
static void RcvHandler(lv_event_t *e);
static void CoinDealHandler(lv_event_t *e);
static void AddFlagCountDownTimerHandler(lv_timer_t *timer);
void AccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count);

static void UpdateManageWalletState(bool needUpdate)
{
    char tempBuf[16] = {0};
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
    sprintf(tempBuf, _("home_select_coin_count_fmt"), selectCnt, total);
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
    label = GuiCreateLittleTitleLabel(g_homeWalletCardCont, "RECEIVE");
    arrow = GuiCreateImg(g_homeWalletCardCont, &imgArrowRight);
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
    label = GuiCreateLittleTitleLabel(g_homeWalletCardCont, "SCAN");
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
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("rcv handler\n");
        GuiFrameOpenViewWithParam(&g_utxoReceiveView, &g_coinCardArray[HOME_WALLET_CARD_BTC], sizeof(g_coinCardArray[HOME_WALLET_CARD_BTC]));
    }
}


static void UpdateHomeConnectWalletCard(void)
{
    lv_obj_t *coinLabel;
    lv_obj_t *chainLabel;
    lv_obj_t *icon;
    lv_obj_t *walletCardCont = g_homeWalletCardCont;
    if (lv_obj_get_child_cnt(walletCardCont) > 0) {
        lv_obj_clean(walletCardCont);
    }

    for (int i = 0, j = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        coinLabel = GuiCreateTextLabel(walletCardCont, g_coinCardArray[i].coin);
        chainLabel = GuiCreateNoticeLabel(walletCardCont, g_coinCardArray[i].chain);
        icon = GuiCreateImg(walletCardCont, g_coinCardArray[i].icon);
        GuiButton_t table[3] = {
            {
                .obj = icon,
                .align = LV_ALIGN_TOP_MID,
                .position = {0, 30},
            },
            {
                .obj = coinLabel,
                .align = LV_ALIGN_TOP_MID,
                .position = {0, 92},
            },
            {
                .obj = chainLabel,
                .align = LV_ALIGN_TOP_MID,
                .position = {0, 130},
            },
        };
        lv_obj_t *button = GuiCreateButton(walletCardCont, 208, 176, table, NUMBER_OF_ARRAYS(table),
                                           CoinDealHandler, (void *) & (g_coinCardArray[i].index));
        lv_obj_align(button, LV_ALIGN_DEFAULT, 24 + ((i - j) % 2) * 224,
                     144 - GUI_MAIN_AREA_OFFSET + ((i - j) / 2) * 192);
        lv_obj_set_style_bg_color(button, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_12, LV_PART_MAIN);
        lv_obj_set_style_radius(button, 24, LV_PART_MAIN);
    }
}


static void CoinDealHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    HOME_WALLET_CARD_ENUM coin;

    if (code == LV_EVENT_CLICKED) {
        coin = *(HOME_WALLET_CARD_ENUM *)lv_event_get_user_data(e);

        switch (coin) {
        case HOME_WALLET_CARD_BTC:
            GuiFrameOpenViewWithParam(&g_utxoReceiveView, &coin, sizeof(coin));
            break;
        default:
            break;
        }
    }
}

static void ManageCoinChainHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        bool state;
        WalletState_t *wallet = lv_event_get_user_data(e);

        lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));
        state = lv_obj_has_state(lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) - 1), LV_STATE_CHECKED);
        g_walletBakState[wallet->index].state = state;
        UpdateManageWalletState(false);
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

        GuiFrameOpenView(&g_scanView);
    }
}

void ConfirmManageAssetsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        UpdateManageWalletState(true);
        //UpdateHomeConnectWalletCard();
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
        GuiFrameOpenView(lv_event_get_user_data(e));
    }
}

static void OpenMoreSettingHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_moreHintbox = GuiCreateHintBox(lv_scr_act(), 480, 228, true);
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

static void OpenManageAssetsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (g_isManageClick == false) {
            return;
        }
        memcpy(&g_walletBakState, &g_walletState, sizeof(g_walletState));
        g_manageCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                          GUI_MAIN_AREA_OFFSET);
        lv_obj_add_flag(g_manageCont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(g_manageCont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
        lv_obj_t *checkBoxCont = GuiCreateContainerWithParent(g_manageCont, lv_obj_get_width(lv_scr_act()), 542);
        lv_obj_set_align(checkBoxCont, LV_ALIGN_DEFAULT);
        lv_obj_add_flag(checkBoxCont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(checkBoxCont, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t *coinLabel;
        lv_obj_t *chainLabel;
        lv_obj_t *icon;
        lv_obj_t *checkbox;
        int heightIndex = 0;
        for (int i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
            coinLabel = GuiCreateTextLabel(checkBoxCont, g_coinCardArray[i].coin);
            chainLabel = GuiCreateNoticeLabel(checkBoxCont, g_coinCardArray[i].chain);
            icon = GuiCreateImg(checkBoxCont, g_coinCardArray[i].icon);
            checkbox = GuiCreateMultiCheckBox(checkBoxCont, _(""));
            lv_obj_set_style_pad_top(checkbox, 32, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_size(checkbox, 446, 96);
            g_walletState[i].checkBox = checkbox;
            uint8_t tableLen = 4;
            GuiButton_t table[4] = {
                {
                    .obj = icon,
                    .align = LV_ALIGN_LEFT_MID,
                    .position = {24, 0},
                },
                {
                    .obj = coinLabel,
                    .align = LV_ALIGN_DEFAULT,
                    .position = {100, 13},
                },
                {
                    .obj = chainLabel,
                    .align = LV_ALIGN_DEFAULT,
                    .position = {100, 53},
                },
                {
                    .obj = checkbox,
                    .align = LV_ALIGN_TOP_MID,
                    .position = {-10, 0},
                },
            };


            lv_obj_t *button = GuiCreateButton(checkBoxCont, 456, 96, table, tableLen,
                                               ManageCoinChainHandler, &g_walletState[i]);
            g_walletButton[i] = button;
            if (!g_walletState[i].enable) {
                lv_obj_add_flag(button, LV_OBJ_FLAG_HIDDEN);
                continue;
            }
            lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 96 * heightIndex);
            heightIndex++;
        }

        lv_obj_t *btn = GuiCreateBtn(g_manageCont, USR_SYMBOL_CHECK);
        lv_obj_add_event_cb(btn, ConfirmManageAssetsHandler, LV_EVENT_ALL, NULL);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);

        lv_obj_t *label = GuiCreateTextLabel(g_manageCont, "");
        lv_obj_align_to(label, btn, LV_ALIGN_OUT_LEFT_MID, -300, 0);
        lv_label_set_recolor(label, true);

        g_manageWalletLabel = label;

        UpdateManageWalletState(false);

        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("home_manage_assets"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnManageWalletHandler, g_manageCont);
        // TODO: add search
        // GuiNvsBarSetRightCb(NVS_BAR_SEARCH, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
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
    lv_obj_set_style_bg_opa(g_pageWidget->contentZone, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_pageWidget->page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_pageWidget->navBar, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_obj_t *walletCardCont = GuiCreateContainerWithParent(g_homeViewCont, lv_obj_get_width(lv_scr_act()),
                               lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(walletCardCont, LV_ALIGN_DEFAULT);
    lv_obj_add_flag(walletCardCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(walletCardCont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(walletCardCont, LV_OPA_TRANSP, LV_PART_MAIN);
    //GuiCreateImg(walletCardCont, &imgDeepLayersVolume11);
    //lv_obj_move_background(walletCardCont);
    g_homeWalletCardCont = walletCardCont;
    CreateHomePageButtons();
    ShowWallPager(true);
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
    ShowWallPager(true);
    g_countDownTimer = lv_timer_create(AddFlagCountDownTimerHandler, 500, NULL);
    GuiSetSetupPhase(SETUP_PAHSE_DONE);
    //if (g_manageCont != NULL) {
    //    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("home_manage_assets"));
    //    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnManageWalletHandler, g_manageCont);
    //    // TODO: add search
    //    // GuiNvsBarSetRightCb(NVS_BAR_SEARCH, NULL, NULL);
    //    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    //} else {
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_MANAGE, OpenManageAssetsHandler, NULL);
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreSettingHandler, NULL);
    //}
    if (g_homeWalletCardCont != NULL) {
        lv_obj_clear_flag(g_homeWalletCardCont, LV_OBJ_FLAG_HIDDEN);
        //lv_obj_clear_flag(g_scanImg, LV_OBJ_FLAG_HIDDEN);
    }
    GUI_DEL_OBJ(g_moreHintbox)
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

#endif
