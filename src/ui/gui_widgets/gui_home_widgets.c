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
#include "gui_qrcode_widgets.h"
#include "gui_web_auth_widgets.h"
#include "gui_setup_widgets.h"
#include "keystore.h"
#include "gui_page.h"

// static uint8_t g_manageWalletNum = 2;
static lv_obj_t *g_manageWalletLabel = NULL;
static lv_obj_t *g_homeWalletCardCont = NULL;
static lv_obj_t *g_homeViewCont = NULL;
static lv_obj_t *g_scanImg = NULL;
static lv_obj_t *g_manageCont = NULL;
static lv_obj_t *g_moreHintbox = NULL;
static bool g_isManageOpen = false;
static PageWidget_t *g_pageWidget;

static WalletState_t g_walletState[HOME_WALLET_CARD_BUTT] = {
    {HOME_WALLET_CARD_BTC, false, "BTC"},
    {HOME_WALLET_CARD_ETH, false, "ETH"},
    {HOME_WALLET_CARD_ATOM, false, "ATOM"},
    {HOME_WALLET_CARD_OSMO, false, "OSMO"},
    {HOME_WALLET_CARD_SCRT, false, "SCRT"},
    {HOME_WALLET_CARD_AKT, false, "AKT"},
    {HOME_WALLET_CARD_CRO, false, "CRO"},
    {HOME_WALLET_CARD_IOV, false, "IOV"},
    {HOME_WALLET_CARD_ROWAN, false, "ROWAN"},
    {HOME_WALLET_CARD_CTK, false, "CTK"},
    {HOME_WALLET_CARD_IRIS, false, "IRIS"},
    {HOME_WALLET_CARD_REGEN, false, "REGEN"},
    {HOME_WALLET_CARD_XPRT, false, "XPRT"},
    {HOME_WALLET_CARD_DVPN, false, "DVPN"},
    {HOME_WALLET_CARD_IXO, false, "IXO"},
    {HOME_WALLET_CARD_NGM, false, "NGM"},
    {HOME_WALLET_CARD_BLD, false, "BLD"},
    {HOME_WALLET_CARD_BOOT, false, "BOOT"},
    {HOME_WALLET_CARD_JUNO, false, "JUNO"},
    {HOME_WALLET_CARD_STARS, false, "STARS"},
    {HOME_WALLET_CARD_AXL, false, "AXL"},
    {HOME_WALLET_CARD_SOMM, false, "SOMM"},
    {HOME_WALLET_CARD_UMEE, false, "UMEE"},
    {HOME_WALLET_CARD_GRAV, false, "GRAV"},
    {HOME_WALLET_CARD_TGD, false, "TGD"},
    {HOME_WALLET_CARD_STRD, false, "STRD"},
    {HOME_WALLET_CARD_EVMOS, false, "EVMOS"},
    {HOME_WALLET_CARD_INJ, false, "INJ"},
    {HOME_WALLET_CARD_KAVA, false, "KAVA"},
    {HOME_WALLET_CARD_QCK, false, "QCK"},
    {HOME_WALLET_CARD_LUNA, false, "LUNA"},
    {HOME_WALLET_CARD_LUNC, false, "LUNC"},
    {HOME_WALLET_CARD_BNB, false, "BNB"},
    {HOME_WALLET_CARD_SOL, false, "SOL"},
    {HOME_WALLET_CARD_DOT, false, "DOT"},
    {HOME_WALLET_CARD_XRP, false, "XRP"},
    {HOME_WALLET_CARD_LTC, false, "LTC"},
    {HOME_WALLET_CARD_DASH, false, "DASH"},
    {HOME_WALLET_CARD_BCH, false, "BCH"},
    {HOME_WALLET_CARD_TRX, false, "TRX"},
};
static WalletState_t g_walletBakState[HOME_WALLET_CARD_BUTT] = {0};

static const ChainCoinCard_t g_coinCardArray[HOME_WALLET_CARD_BUTT] = {
    {
        .index = HOME_WALLET_CARD_BTC,
        .coin = "BTC",
        .chain = "Bitcoin",
        .icon = &coinBtc,
    },
    {
        .index = HOME_WALLET_CARD_ETH,
        .coin = "ETH",
        .chain = "Ethereum",
        .icon = &coinEth,
    },
    {
        .index = HOME_WALLET_CARD_ATOM,
        .coin = "ATOM",
        .chain = "Cosmos Hub",
        .icon = &coinAtom,
    },
    {
        .index = HOME_WALLET_CARD_OSMO,
        .coin = "OSMO",
        .chain = "Osmosis",
        .icon = &coinOsmo,
    },
    {
        .index = HOME_WALLET_CARD_SCRT,
        .coin = "SCRT",
        .chain = "Secret Network",
        .icon = &coinScrt,
    },
    {
        .index = HOME_WALLET_CARD_AKT,
        .coin = "AKT",
        .chain = "Akash",
        .icon = &coinAkt,
    },
    {
        .index = HOME_WALLET_CARD_CRO,
        .coin = "CRO",
        .chain = "Crypto.org",
        .icon = &coinCro,
    },
    {
        .index = HOME_WALLET_CARD_IOV,
        .coin = "IOV",
        .chain = "Starname",
        .icon = &coinIov,
    },
    {
        .index = HOME_WALLET_CARD_ROWAN,
        .coin = "ROWAN",
        .chain = "Sifchain",
        .icon = &coinRowan,
    },
    {
        .index = HOME_WALLET_CARD_CTK,
        .coin = "CTK",
        .chain = "Shentu",
        .icon = &coinCtk,
    },
    {
        .index = HOME_WALLET_CARD_IRIS,
        .coin = "IRIS",
        .chain = "IRISnet",
        .icon = &coinIris,
    },
    {
        .index = HOME_WALLET_CARD_REGEN,
        .coin = "REGEN",
        .chain = "Regen",
        .icon = &coinRegen,
    },
    {
        .index = HOME_WALLET_CARD_XPRT,
        .coin = "XPRT",
        .chain = "Persistence",
        .icon = &coinXprt,
    },
    {
        .index = HOME_WALLET_CARD_DVPN,
        .coin = "DVPN",
        .chain = "Sentinel",
        .icon = &coinDvpn,
    },
    {
        .index = HOME_WALLET_CARD_IXO,
        .coin = "IXO",
        .chain = "ixo",
        .icon = &coinIxo,
    },
    {
        .index = HOME_WALLET_CARD_NGM,
        .coin = "NGM",
        .chain = "e-Money",
        .icon = &coinNgm,
    },
    {
        .index = HOME_WALLET_CARD_BLD,
        .coin = "BLD",
        .chain = "Agoric",
        .icon = &coinBld,
    },
    {
        .index = HOME_WALLET_CARD_BOOT,
        .coin = "BOOT",
        .chain = "Bostrom",
        .icon = &coinBoot,
    },
    {
        .index = HOME_WALLET_CARD_JUNO,
        .coin = "JUNO",
        .chain = "Juno",
        .icon = &coinJuno,
    },
    {
        .index = HOME_WALLET_CARD_STARS,
        .coin = "STARS",
        .chain = "Stargaze",
        .icon = &coinStars,
    },
    {
        .index = HOME_WALLET_CARD_AXL,
        .coin = "AXL",
        .chain = "Axelar",
        .icon = &coinAxl,
    },
    {
        .index = HOME_WALLET_CARD_SOMM,
        .coin = "SOMM",
        .chain = "Sommelier",
        .icon = &coinSomm,
    },
    {
        .index = HOME_WALLET_CARD_UMEE,
        .coin = "UMEE",
        .chain = "Umee",
        .icon = &coinUmee,
    },
    {
        .index = HOME_WALLET_CARD_GRAV,
        .coin = "GRAV",
        .chain = "Gravity Bridge",
        .icon = &coinGrav,
    },
    {
        .index = HOME_WALLET_CARD_TGD,
        .coin = "TGD",
        .chain = "Tgrade",
        .icon = &coinTgd,
    },
    {
        .index = HOME_WALLET_CARD_STRD,
        .coin = "STRD",
        .chain = "Stride",
        .icon = &coinStrd,
    },
    {
        .index = HOME_WALLET_CARD_EVMOS,
        .coin = "EVMOS",
        .chain = "Evmos",
        .icon = &coinEvmos,
    },
    {
        .index = HOME_WALLET_CARD_INJ,
        .coin = "INJ",
        .chain = "Injective",
        .icon = &coinInj,
    },
    {
        .index = HOME_WALLET_CARD_KAVA,
        .coin = "KAVA",
        .chain = "Kava",
        .icon = &coinKava,
    },
    {
        .index = HOME_WALLET_CARD_QCK,
        .coin = "QCK",
        .chain = "Quicksilver",
        .icon = &coinQck,
    },
    {
        .index = HOME_WALLET_CARD_LUNA,
        .coin = "LUNA",
        .chain = "Terra",
        .icon = &coinLuna,
    },
    {
        .index = HOME_WALLET_CARD_LUNC,
        .coin = "LUNC",
        .chain = "Terra Classic",
        .icon = &coinLunc,
    },
    {
        .index = HOME_WALLET_CARD_BNB,
        .coin = "BNB",
        .chain = "Binance",
        .icon = &coinBnb,
    },
    {
        .index = HOME_WALLET_CARD_SOL,
        .coin = "SOL",
        .chain = "Solana",
        .icon = &coinSol,
    },
    {
        .index = HOME_WALLET_CARD_TRX,
        .coin = "TRX",
        .chain = "TRON",
        .icon = &coinTrx,
    },
    {
        .index = HOME_WALLET_CARD_DOT,
        .coin = "DOT",
        .chain = "Polkadot",
        .icon = &coinDot,
    },
    {
        .index = HOME_WALLET_CARD_XRP,
        .coin = "XRP",
        .chain = "Ripple",
        .icon = &coinXrp,
    },
    {
        .index = HOME_WALLET_CARD_LTC,
        .coin = "LTC",
        .chain = "Litecoin",
        .icon = &coinLtc,
    },
    {
        .index = HOME_WALLET_CARD_DASH,
        .coin = "DASH",
        .chain = "Dash",
        .icon = &coinDash,
    },
    {
        .index = HOME_WALLET_CARD_BCH,
        .coin = "BCH",
        .chain = "Bitcoin Cash",
        .icon = &coinBch,
    },
};

static void CoinDealHandler(lv_event_t *e);
static bool IsUtxoCoin(HOME_WALLET_CARD_ENUM coin);
void AccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count);
void AccountPublicHomeCoinGet(WalletState_t *walletList, uint8_t count);

static void UpdateManageWalletState(bool needUpdate)
{
    char tempBuf[16] = {0};
    uint8_t selectCnt = 0;
    g_isManageOpen = false;

    //only show btc, eth now
    for (int i = 0; i < HOME_WALLET_CARD_BNB; i++) {
        if (g_walletBakState[i].state == true) {
            selectCnt++;
            lv_obj_add_state(g_walletState[i].checkBox, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(g_walletState[i].checkBox, LV_STATE_CHECKED);
        }
    }
    sprintf(tempBuf, "#F5870A %d#/%d", selectCnt, HOME_WALLET_CARD_BNB);
    lv_label_set_text(g_manageWalletLabel, tempBuf);
    if (needUpdate) {
        if (memcmp(g_walletState, g_walletBakState, sizeof(g_walletState))) {
            memcpy(g_walletState, g_walletBakState, sizeof(g_walletBakState));
            AccountPublicHomeCoinSet(g_walletState, NUMBER_OF_ARRAYS(g_walletState));
        }
        // g_manageWalletNum = selectCnt;
    }
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
        if (g_walletState[i].state == false) {
            j++;
            continue;
        }
        coinLabel = GuiCreateTextLabel(walletCardCont, g_coinCardArray[i].coin);
        chainLabel = GuiCreateNoticeLabel(walletCardCont, g_coinCardArray[i].chain);
        icon = GuiCreateImg(walletCardCont, g_coinCardArray[i].icon);
        GuiButton_t table[3] = {
            {.obj = icon, .align = LV_ALIGN_TOP_MID, .position = {0, 30},},
            {.obj = coinLabel, .align = LV_ALIGN_TOP_MID, .position = {0, 92},},
            {.obj = chainLabel, .align = LV_ALIGN_TOP_MID, .position = {0, 130},},
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


static bool IsUtxoCoin(HOME_WALLET_CARD_ENUM coin)
{
    switch (coin) {
    case HOME_WALLET_CARD_BTC:
    case HOME_WALLET_CARD_LTC:
    case HOME_WALLET_CARD_DASH:
    case HOME_WALLET_CARD_BCH:
        return true;
    default:
        return false;
    }
}

static void CoinDealHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    HOME_WALLET_CARD_ENUM coin;

    if (code == LV_EVENT_CLICKED) {
        coin = *(HOME_WALLET_CARD_ENUM *)lv_event_get_user_data(e);

        if (IsUtxoCoin(coin)) {
            GuiFrameOpenViewWithParam(&g_utxoReceiveView, &coin, sizeof(coin));
        } else if (coin == HOME_WALLET_CARD_ETH) {
            GuiFrameOpenView(&g_ethereumReceiveView);
        } else {
            GuiFrameOpenViewWithParam(&g_standardReceiveView, &coin, sizeof(coin));
        }
    }
}

static void ManageCoinChainHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        WalletState_t *wallet = lv_event_get_user_data(e);
        lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));
        bool state = lv_obj_has_state(lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) - 1), LV_STATE_CHECKED);
        g_walletBakState[wallet->index].state = state;

        lv_obj_scroll_to_y(lv_obj_get_parent(parent), (wallet->index - 2) * 96, LV_ANIM_ON);
        UpdateManageWalletState(false);
    }
}

void ScanQrCodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenView(lv_event_get_user_data(e));
    }
}

void ConfirmManageAssetsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        UpdateManageWalletState(true);
        UpdateHomeConnectWalletCard();
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
            {.obj = img, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
            {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {76, 0},},
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
        lv_obj_t *tag;

        //only show btc eth now;
        for (int i = 0; i < HOME_WALLET_CARD_BNB; i++) {
            coinLabel = GuiCreateTextLabel(checkBoxCont, g_coinCardArray[i].coin);
            chainLabel = GuiCreateNoticeLabel(checkBoxCont, g_coinCardArray[i].chain);
            icon = GuiCreateImg(checkBoxCont, g_coinCardArray[i].icon);
            checkbox = GuiCreateMultiCheckBox(checkBoxCont, _(""));
            g_walletState[i].checkBox = checkbox;
            uint8_t tableLen = 4;
            GuiButton_t table[5] = {
                {.obj = icon, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
                {.obj = coinLabel, .align = LV_ALIGN_DEFAULT, .position = {100, 13},},
                {.obj = chainLabel, .align = LV_ALIGN_DEFAULT, .position = {100, 53},},
                {.obj = checkbox, .align = LV_ALIGN_TOP_MID, .position = {0, 32},},
            };
            if (IsCosmosChain(g_coinCardArray[i].index)) {
                tag = GuiCreateImg(checkBoxCont, &imgCosmosTag);
                table[4] = table[3];
                table[3].obj = tag;
                table[3].align = LV_ALIGN_LEFT_MID;
                GuiPosition_t p = {272, 0};
                table[3].position = p;
                tableLen = 5;
            }
            lv_obj_t *button = GuiCreateButton(checkBoxCont, 456, 96, table, tableLen,
                                               ManageCoinChainHandler, &g_walletState[i]);
            lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 96 * i);
        }

        lv_obj_t *btn = GuiCreateBtn(g_manageCont, USR_SYMBOL_CHECK);
        lv_obj_add_event_cb(btn, ConfirmManageAssetsHandler, LV_EVENT_ALL, NULL);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);

        lv_obj_t *label = GuiCreateTextLabel(g_manageCont, "");
        lv_obj_align_to(label, btn, LV_ALIGN_OUT_LEFT_MID, -300, 0);
        lv_label_set_recolor(label, true);

        g_manageWalletLabel = label;

        // ** temporary add.
        lv_obj_t *line = GuiCreateDividerLine(checkBoxCont);
        lv_obj_align(line, LV_ALIGN_DEFAULT, 0, HOME_WALLET_CARD_BNB * 96);

        label = GuiCreateIllustrateLabel(checkBoxCont, "Please upgrade to the latest version for access to expanded crypto compatibility.");
        lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 32, HOME_WALLET_CARD_BNB * 96 + 25);
        // **
        UpdateManageWalletState(false);

        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Manage Assets"));
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

    lv_obj_t *walletCardCont = GuiCreateContainerWithParent(g_homeViewCont, lv_obj_get_width(lv_scr_act()),
                               lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(walletCardCont, LV_ALIGN_DEFAULT);
    lv_obj_add_flag(walletCardCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(walletCardCont, LV_SCROLLBAR_MODE_OFF);
    g_homeWalletCardCont = walletCardCont;

    lv_obj_t *img = GuiCreateImg(lv_scr_act(), &imgScan);
    lv_obj_align(img, LV_ALIGN_BOTTOM_RIGHT, -32, -32);
    lv_obj_add_event_cb(img, ScanQrCodeHandler, LV_EVENT_CLICKED, &g_qrCodeView);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    g_scanImg = img;
}

void GuiHomeDisActive(void)
{
    if (g_homeWalletCardCont != NULL) {
        lv_obj_add_flag(g_homeWalletCardCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_scanImg, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiHomeRestart(void)
{
    GUI_DEL_OBJ(g_manageCont)
    GuiHomeRefresh();
}

void GuiHomeRefresh(void)
{
    printf("%s %d\n", __func__, __LINE__);
    if (GetCurrentAccountIndex() > 2) {
        return;
    }
    GuiSetSetupPhase(SETUP_PAHSE_DONE);
    if (g_manageCont != NULL) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Manage Assets"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnManageWalletHandler, g_manageCont);
        // TODO: add search
        // GuiNvsBarSetRightCb(NVS_BAR_SEARCH, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    } else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_MANAGE, OpenManageAssetsHandler, NULL);
        SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreSettingHandler, NULL);
    }
    if (g_homeWalletCardCont != NULL) {
        lv_obj_clear_flag(g_homeWalletCardCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_scanImg, LV_OBJ_FLAG_HIDDEN);
    }
    GUI_DEL_OBJ(g_moreHintbox)
    AccountPublicHomeCoinGet(g_walletState, NUMBER_OF_ARRAYS(g_walletState));
    UpdateHomeConnectWalletCard();
}

const ChainCoinCard_t* GetCoinCardByIndex(HOME_WALLET_CARD_ENUM index)
{
    for (int i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        if (g_coinCardArray[i].index == index) {
            return &g_coinCardArray[i];
        }
    }
    return NULL;
}
