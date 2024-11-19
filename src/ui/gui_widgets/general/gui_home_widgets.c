#ifndef BTC_ONLY
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
#include "version.h"
#include "gui_pending_hintbox.h"

typedef enum {
    GestureRight = 1,
    GestureLeft = -1,
    GestureNone = 0,
} HomeGesture_t;

typedef enum {
    COIN_FILTER_MAIN = 0,
    COIN_FILTER_COSMOS = 1,

    COIN_FILTER_BUTT,
} CoinFilter_t;

typedef struct {
    void (*OnSwipeLeft)(HomeGesture_t gesture);
    void (*OnSwipeRight)(HomeGesture_t gesture);
    void (*OnClick)(lv_event_t *e);
} ScrollHandler_t;

#define CARDS_PER_PAGE                      8
#define COIN_PER_PAGE                       5
#define EACH_CARD_WIDTH                     208
#define EACH_CARD_HEIGHT                    128
#define EACH_CARD_HORIZONTAL_SPACING        16
#define EACH_CARD_VERTICAL_SPACING          16
#define EACH_PAGE_LED_HORIZONTAL_SPACING    12

static lv_obj_t *g_manageWalletLabel = NULL;
static lv_obj_t *g_homeWalletCardCont = NULL;
static lv_obj_t *g_homeViewCont = NULL;
static lv_obj_t *g_scanImg = NULL;
static lv_obj_t *g_manageCont = NULL;
static lv_obj_t *g_moreHintbox = NULL;
static bool g_isManageOpen = false;
static bool g_isManageClick = true;
static PageWidget_t *g_pageWidget;
static lv_timer_t *g_countDownTimer = NULL; // count down timer
static lv_obj_t *g_walletButton[HOME_WALLET_CARD_BUTT];
static lv_obj_t *g_noticeWindow = NULL;
static uint8_t g_currentPage = 0;
static uint8_t g_coinCurrentPage = 0;
static bool g_isScrolling = false;
const static char *g_coinFilter[] = {"Main Chain", "COSMOS Eco"};
static uint8_t g_currentFilter = COIN_FILTER_MAIN;
static uint8_t g_coinFilterNum = 0;
static lv_obj_t *g_coinListCont = NULL;

static WalletState_t g_walletState[HOME_WALLET_CARD_BUTT] = {
    {HOME_WALLET_CARD_BTC, false, "BTC", true},
    {HOME_WALLET_CARD_ETH, false, "ETH", true},
    {HOME_WALLET_CARD_SOL, false, "SOL", true},
    {HOME_WALLET_CARD_BNB, false, "BNB", false},
    {HOME_WALLET_CARD_HNT, false, "HNT", true},
    {HOME_WALLET_CARD_XRP, false, "XRP", true},
    {HOME_WALLET_CARD_ADA, false, "ADA", true},
    {HOME_WALLET_CARD_TON, false, "TON", false},
    {HOME_WALLET_CARD_DOT, false, "DOT", false},
    {HOME_WALLET_CARD_TRX, false, "TRX", true},
    {HOME_WALLET_CARD_LTC, false, "LTC", true},
    {HOME_WALLET_CARD_BCH, false, "BCH", true},
    {HOME_WALLET_CARD_APT, false, "APT", true},
    {HOME_WALLET_CARD_SUI, false, "SUI", true},
    {HOME_WALLET_CARD_DASH, false, "DASH", true},
    {HOME_WALLET_CARD_ARWEAVE, false, "AR", true},
    {HOME_WALLET_CARD_XLM, false, "XLM", true},
    {HOME_WALLET_CARD_TIA, false, "TIA", true},
    {HOME_WALLET_CARD_DYM, false, "DYM", true},
    {HOME_WALLET_CARD_OSMO, false, "OSMO", true},
    {HOME_WALLET_CARD_INJ, false, "INJ", true},
    {HOME_WALLET_CARD_ATOM, false, "ATOM", true},
    {HOME_WALLET_CARD_CRO, false, "CRO", true},
    {HOME_WALLET_CARD_RUNE, false, "RUNE", true},
    {HOME_WALLET_CARD_KAVA, false, "KAVA", true},
    {HOME_WALLET_CARD_LUNC, false, "LUNC", true},
    {HOME_WALLET_CARD_AXL, false, "AXL", true},
    {HOME_WALLET_CARD_LUNA, false, "LUNA", true},
    {HOME_WALLET_CARD_AKT, false, "AKT", true},
    {HOME_WALLET_CARD_STRD, false, "STRD", true},
    {HOME_WALLET_CARD_SCRT, false, "SCRT", true},
    {HOME_WALLET_CARD_BLD, false, "BLD", true},
    {HOME_WALLET_CARD_CTK, false, "CTK", true},
    {HOME_WALLET_CARD_EVMOS, false, "EVMOS", true},
    {HOME_WALLET_CARD_STARS, false, "STARS", true},
    {HOME_WALLET_CARD_XPRT, false, "XPRT", true},
    {HOME_WALLET_CARD_SOMM, false, "SOMM", true},
    {HOME_WALLET_CARD_JUNO, false, "JUNO", true},
    {HOME_WALLET_CARD_IRIS, false, "IRIS", true},
    {HOME_WALLET_CARD_DVPN, false, "DVPN", true},
    {HOME_WALLET_CARD_ROWAN, false, "ROWAN", true},
    {HOME_WALLET_CARD_REGEN, false, "REGEN", true},
    {HOME_WALLET_CARD_BOOT, false, "BOOT", true},
    {HOME_WALLET_CARD_GRAV, false, "GRAV", true},
    {HOME_WALLET_CARD_IXO, false, "IXO", true},
    {HOME_WALLET_CARD_NGM, false, "NGM", true},
    {HOME_WALLET_CARD_IOV, false, "IOV", true},
    {HOME_WALLET_CARD_UMEE, false, "UMEE", true},
    {HOME_WALLET_CARD_QCK, false, "QCK", true},
    {HOME_WALLET_CARD_TGD, false, "TGD", true},
};
static WalletState_t g_walletBakState[HOME_WALLET_CARD_BUTT] = {0};
static KeyboardWidget_t *g_keyboardWidget = NULL;

static void GuiInitWalletState()
{
    for (size_t i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        g_walletState[i].enable = false;
        g_walletState[i].state = false;
    }
    MnemonicType mnemonicType = GetMnemonicType();
    switch (mnemonicType) {
    case MNEMONIC_TYPE_SLIP39:
        for (size_t i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
            g_walletState[i].enable = true;
        }
        g_walletState[HOME_WALLET_CARD_BNB].enable = false;
        g_walletState[HOME_WALLET_CARD_DOT].enable = false;
        g_walletState[HOME_WALLET_CARD_ADA].enable = false;
        g_walletState[HOME_WALLET_CARD_TON].enable = true;
        g_coinFilterNum = 2;
        break;
    case MNEMONIC_TYPE_BIP39:
        for (size_t i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
            g_walletState[i].enable = true;
        }
        g_walletState[HOME_WALLET_CARD_BNB].enable = false;
        g_walletState[HOME_WALLET_CARD_DOT].enable = false;
        g_walletState[HOME_WALLET_CARD_ADA].enable = true;
        g_walletState[HOME_WALLET_CARD_TON].enable = true;
        g_coinFilterNum = 2;
        break;
    default:
        g_walletState[HOME_WALLET_CARD_TON].enable = true;
        g_walletState[HOME_WALLET_CARD_TON].state = true;
        g_coinFilterNum = 0;
        break;
    }
}

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
        .index = HOME_WALLET_CARD_SOL,
        .coin = "SOL",
        .chain = "Solana",
        .icon = &coinSol,
    },
    {
        .index = HOME_WALLET_CARD_BNB,
        .coin = "BNB",
        .chain = "Binance",
        .icon = &coinBnb,
    },
    {
        .index = HOME_WALLET_CARD_HNT,
        .coin = "HNT",
        .chain = "Helium",
        .icon = &coinHelium,
    },
    {
        .index = HOME_WALLET_CARD_XRP,
        .coin = "XRP",
        .chain = "Ripple",
        .icon = &coinXrp,
    },
    {
        .index = HOME_WALLET_CARD_ADA,
        .coin = "ADA",
        .chain = "Cardano",
        .icon = &coinAda,
    },
    {
        .index = HOME_WALLET_CARD_TON,
        .coin = "TON",
        .chain = "TON-TON",
        .icon = &coinTon,
    },
    {
        .index = HOME_WALLET_CARD_DOT,
        .coin = "DOT",
        .chain = "Polkadot",
        .icon = &coinDot,
    },
    {
        .index = HOME_WALLET_CARD_TRX,
        .coin = "TRX",
        .chain = "TRON",
        .icon = &coinTrx,
    },
    {
        .index = HOME_WALLET_CARD_LTC,
        .coin = "LTC",
        .chain = "Litecoin",
        .icon = &coinLtc,
    },
    {
        .index = HOME_WALLET_CARD_BCH,
        .coin = "BCH",
        .chain = "Bitcoin Cash",
        .icon = &coinBch,
    },
    {
        .index = HOME_WALLET_CARD_APT,
        .coin = "APT",
        .chain = "Aptos",
        .icon = &coinApt,
    },
    {
        .index = HOME_WALLET_CARD_SUI,
        .coin = "SUI",
        .chain = "Sui",
        .icon = &coinSui,
    },
    {
        .index = HOME_WALLET_CARD_DASH,
        .coin = "DASH",
        .chain = "Dash",
        .icon = &coinDash,
    },
    {
        .index = HOME_WALLET_CARD_ARWEAVE,
        .coin = "AR",
        .chain = "Arweave",
        .icon = &coinAr,
    },
    {
        .index = HOME_WALLET_CARD_XLM,
        .coin = "XLM",
        .chain = "Stellar",
        .icon = &coinXlm,
    },
    {
        .index = HOME_WALLET_CARD_TIA,
        .coin = "TIA",
        .chain = "Celestia",
        .icon = &coinTia,
    },
    {
        .index = HOME_WALLET_CARD_DYM,
        .coin = "DYM",
        .chain = "Dymension",
        .icon = &coinDym,
    },
    {
        .index = HOME_WALLET_CARD_OSMO,
        .coin = "OSMO",
        .chain = "Osmosis",
        .icon = &coinOsmo,
    },
    {
        .index = HOME_WALLET_CARD_INJ,
        .coin = "INJ",
        .chain = "Injective",
        .icon = &coinInj,
    },
    {
        .index = HOME_WALLET_CARD_ATOM,
        .coin = "ATOM",
        .chain = "Cosmos Hub",
        .icon = &coinAtom,
    },
    {
        .index = HOME_WALLET_CARD_CRO,
        .coin = "CRO",
        .chain = "CRO-Cronos",
        .icon = &coinCro,
    },
    {
        .index = HOME_WALLET_CARD_RUNE,
        .coin = "RUNE",
        .chain = "THORChain",
        .icon = &coinRune,
    },
    {
        .index = HOME_WALLET_CARD_KAVA,
        .coin = "KAVA",
        .chain = "Kava",
        .icon = &coinKava,
    },
    {
        .index = HOME_WALLET_CARD_LUNC,
        .coin = "LUNC",
        .chain = "Terra Classic",
        .icon = &coinLunc,
    },
    {
        .index = HOME_WALLET_CARD_AXL,
        .coin = "AXL",
        .chain = "Axelar",
        .icon = &coinAxl,
    },
    {
        .index = HOME_WALLET_CARD_LUNA,
        .coin = "LUNA",
        .chain = "Terra",
        .icon = &coinLuna,
    },
    {
        .index = HOME_WALLET_CARD_AKT,
        .coin = "AKT",
        .chain = "Akash",
        .icon = &coinAkt,
    },
    {
        .index = HOME_WALLET_CARD_STRD,
        .coin = "STRD",
        .chain = "Stride",
        .icon = &coinStrd,
    },
    {
        .index = HOME_WALLET_CARD_SCRT,
        .coin = "SCRT",
        .chain = "SCRT-Secret",
        .icon = &coinScrt,
    },
    {
        .index = HOME_WALLET_CARD_BLD,
        .coin = "BLD",
        .chain = "Agoric",
        .icon = &coinBld,
    },
    {
        .index = HOME_WALLET_CARD_CTK,
        .coin = "CTK",
        .chain = "Shentu",
        .icon = &coinCtk,
    },
    {
        .index = HOME_WALLET_CARD_EVMOS,
        .coin = "EVMOS",
        .chain = "Evmos",
        .icon = &coinEvmos,
    },
    {
        .index = HOME_WALLET_CARD_STARS,
        .coin = "STARS",
        .chain = "Stargaze",
        .icon = &coinStars,
    },
    {
        .index = HOME_WALLET_CARD_XPRT,
        .coin = "XPRT",
        .chain = "Persistence",
        .icon = &coinXprt,
    },
    {
        .index = HOME_WALLET_CARD_SOMM,
        .coin = "SOMM",
        .chain = "Sommelier",
        .icon = &coinSomm,
    },
    {
        .index = HOME_WALLET_CARD_JUNO,
        .coin = "JUNO",
        .chain = "Juno",
        .icon = &coinJuno,
    },
    {
        .index = HOME_WALLET_CARD_IRIS,
        .coin = "IRIS",
        .chain = "IRISnet",
        .icon = &coinIris,
    },
    {
        .index = HOME_WALLET_CARD_DVPN,
        .coin = "DVPN",
        .chain = "Sentinel",
        .icon = &coinDvpn,
    },
    {
        .index = HOME_WALLET_CARD_ROWAN,
        .coin = "ROWAN",
        .chain = "Sifchain",
        .icon = &coinRowan,
    },
    {
        .index = HOME_WALLET_CARD_REGEN,
        .coin = "REGEN",
        .chain = "Regen",
        .icon = &coinRegen,
    },
    {
        .index = HOME_WALLET_CARD_BOOT,
        .coin = "BOOT",
        .chain = "Bostrom",
        .icon = &coinBoot,
    },
    {
        .index = HOME_WALLET_CARD_GRAV,
        .coin = "GRAV",
        .chain = "GRAV-Graviton",
        .icon = &coinGrav,
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
        .index = HOME_WALLET_CARD_IOV,
        .coin = "IOV",
        .chain = "Starname",
        .icon = &coinIov,
    },
    {
        .index = HOME_WALLET_CARD_UMEE,
        .coin = "UMEE",
        .chain = "Umee",
        .icon = &coinUmee,
    },
    {
        .index = HOME_WALLET_CARD_QCK,
        .coin = "QCK",
        .chain = "Quicksilver",
        .icon = &coinQck,
    },
    {
        .index = HOME_WALLET_CARD_TGD,
        .coin = "TGD",
        .chain = "Tgrade",
        .icon = &coinTgd,
    },

};

static void CoinDealHandler(HOME_WALLET_CARD_ENUM coin);
static void AddFlagCountDownTimerHandler(lv_timer_t *timer);
void AccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count);
static void CloseArHintbox(void);
static void ResetScrollState(lv_timer_t * timer);
static void HomeScrollHandler(lv_event_t * e);
static void CoinScrollHandler(lv_event_t * e);

static void CloseArHintbox(void)
{
    GuiCloseAttentionHintbox();
    if (g_keyboardWidget != NULL) {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
    }
}

static uint8_t GetSelectedWalletCount(void)
{
    uint8_t selectCnt = 0;
    for (int i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        if (GetIsTempAccount() && g_walletState[i].index == HOME_WALLET_CARD_ARWEAVE) {
            continue;
        }

        if (g_walletState[i].state == true) {
            selectCnt++;
        }
    }
    return selectCnt;
}

static void GuiDrawPageSelectLed(lv_obj_t *parent, int numPoints, int onLedIndex)
{
    if (numPoints < 2) {
        return;
    }
    int startX = (480 - (numPoints - 1) * EACH_PAGE_LED_HORIZONTAL_SPACING) / 2;
    for (int i = 0; i < numPoints; i++) {
        int x = startX + (i * EACH_PAGE_LED_HORIZONTAL_SPACING);
        lv_obj_t *led = lv_led_create(parent);
        lv_led_set_brightness(led, 115);
        lv_obj_set_style_shadow_width(led, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
        lv_led_set_color(led, WHITE_COLOR);
        if (i == onLedIndex) {
            lv_led_on(led);
        }
        lv_obj_set_size(led, 6, 6);
        lv_obj_set_style_radius(led, 0, LV_PART_MAIN);
        lv_obj_align(led, LV_ALIGN_DEFAULT, x, 0);
    }
}

static void UpdateManageWalletState(bool needUpdate)
{
    char tempBuf[BUFFER_SIZE_32] = {0};
    uint8_t selectCnt = 0;
    g_isManageOpen = false;
    int total = 0;
    for (int i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        if (GetIsTempAccount() && g_walletState[i].index == HOME_WALLET_CARD_ARWEAVE) {
            continue;
        }

        if (g_walletState[i].enable) {
            total++;
        }
        if (g_walletBakState[i].state == true) {
            selectCnt++;
            if (g_walletState[i].checkBox != NULL) {
                lv_obj_add_state(g_walletState[i].checkBox, LV_STATE_CHECKED);
            }
        } else {
            if (g_walletState[i].checkBox != NULL) {
                lv_obj_clear_state(g_walletState[i].checkBox, LV_STATE_CHECKED);
            }
        }
    }
    snprintf_s(tempBuf, sizeof(tempBuf), _("home_select_coin_count_fmt"), selectCnt, total);
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
    UpdateManageWalletState(false);
    GUI_DEL_OBJ(g_manageCont);
    g_currentFilter = COIN_FILTER_MAIN;
    GuiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
}

static bool UpdateStartIndex(int8_t gesture, uint8_t totalCount)
{
    int8_t newPage = g_currentPage + gesture;

    if (newPage < 0 || newPage * CARDS_PER_PAGE >= totalCount) {
        return false;
    }

    g_currentPage = newPage;
    return true;
}

static bool UpdateCoinStartIndex(int8_t gesture, uint8_t totalCount)
{
    int8_t newPage = g_coinCurrentPage + gesture;

    if (newPage < 0 || newPage * COIN_PER_PAGE >= totalCount) {
        return false;
    }

    g_coinCurrentPage = newPage;
    return true;
}

static void UpdateHomeConnectWalletCard(HomeGesture_t gesture)
{
    lv_obj_t *walletCardCont = g_homeWalletCardCont;
    if (lv_obj_get_child_cnt(walletCardCont) > 0) {
        lv_obj_clean(walletCardCont);
    }
    uint8_t currentCoinAmount = 0;
    uint8_t totalCoinAmount = 0xFF;
    totalCoinAmount = GetSelectedWalletCount();
    UpdateStartIndex(gesture, totalCoinAmount);

    lv_obj_t *pageSelectCont = GuiCreateContainerWithParent(walletCardCont, 480, 6);
    lv_obj_align(pageSelectCont, LV_ALIGN_BOTTOM_MID, 0, -17);
    GuiDrawPageSelectLed(pageSelectCont, (totalCoinAmount % CARDS_PER_PAGE == 0) ? (totalCoinAmount / CARDS_PER_PAGE) : (totalCoinAmount / CARDS_PER_PAGE + 1), g_currentPage);

    for (int i = 0, j = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        if ((g_walletState[i].index == HOME_WALLET_CARD_ARWEAVE && GetIsTempAccount()) ||
                g_walletState[i].state == false ||
                g_walletState[i].enable == false) {
            j++;
            continue;
        }

        if ((i - j) < g_currentPage * CARDS_PER_PAGE) {
            continue;
        }

        lv_obj_t *coinLabel = GuiCreateTextLabel(walletCardCont, g_coinCardArray[i].coin);
        lv_obj_t *chainLabel = GuiCreateNoticeLabel(walletCardCont, g_coinCardArray[i].chain);
        lv_obj_t *icon = GuiCreateImg(walletCardCont, g_coinCardArray[i].icon);
        GuiButton_t table[3] = {
            {.obj = icon, .align = LV_ALIGN_LEFT_MID, .position = {16, 0},},
            {.obj = coinLabel, .align = LV_ALIGN_DEFAULT, .position = {80, 33},},
            {.obj = chainLabel, .align = LV_ALIGN_DEFAULT, .position = {80, 71},},
        };
        lv_obj_t *button = GuiCreateButton(walletCardCont, EACH_CARD_WIDTH, EACH_CARD_HEIGHT, table, NUMBER_OF_ARRAYS(table),
                                           NULL, (void *) & (g_coinCardArray[i].index));
        lv_obj_add_event_cb(button, HomeScrollHandler, LV_EVENT_ALL, (void *) & (g_coinCardArray[i].index));
        lv_obj_clear_flag(button, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 24 + ((i - j) % 2) * 224,
                     144 - GUI_MAIN_AREA_OFFSET + (((i - j) % CARDS_PER_PAGE) / 2) * (EACH_CARD_HEIGHT + EACH_CARD_VERTICAL_SPACING));
        lv_obj_set_style_bg_color(button, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_12, LV_PART_MAIN);
        lv_obj_set_style_radius(button, 24, LV_PART_MAIN);
        if (currentCoinAmount++ == (CARDS_PER_PAGE - 1)) {
            break;
        }
    }
}

void GuiShowRsaSetupasswordHintbox(void)
{
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void GuiARAddressCheckConfirmHandler(lv_event_t *event)
{
    GUI_DEL_OBJ(g_noticeWindow);
    GuiCreateAttentionHintbox(SIG_SETUP_RSA_PRIVATE_KEY_RECEIVE_CONFIRM);
}

static void GuiOpenARAddressNoticeWindow()
{
    g_noticeWindow = GuiCreateGeneralHintBox(&imgWarn, _("ar_address_check"), _("ar_address_check_desc"), NULL, _("Not Now"), WHITE_COLOR_OPA20, _("Understand"), ORANGE_COLOR);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeWindow, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_set_width(btn, 192);
    lv_obj_set_style_text_font(lv_obj_get_child(btn, 0), &buttonFont, 0);
    lv_obj_add_event_cb(btn, GuiARAddressCheckConfirmHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    btn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_set_width(btn, 192);
    lv_obj_set_style_text_font(lv_obj_get_child(btn, 0), &buttonFont, 0);
    lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgClose);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_align_to(img, lv_obj_get_child(g_noticeWindow, 1), LV_ALIGN_TOP_RIGHT, -36, 36);
}

static void CoinDealHandler(HOME_WALLET_CARD_ENUM coin)
{
    if (coin >= HOME_WALLET_CARD_BUTT) {
        return;
    }
    switch (coin) {
    case HOME_WALLET_CARD_BTC:
    case HOME_WALLET_CARD_LTC:
    case HOME_WALLET_CARD_DASH:
    case HOME_WALLET_CARD_BCH:
        GuiFrameOpenViewWithParam(&g_utxoReceiveView, &coin, sizeof(coin));
        break;
    case HOME_WALLET_CARD_ETH:
    case HOME_WALLET_CARD_SOL:
    case HOME_WALLET_CARD_HNT:
        GuiFrameOpenViewWithParam(&g_multiPathCoinReceiveView, &coin, sizeof(coin));
        break;
    case HOME_WALLET_CARD_ADA:
        GuiFrameOpenViewWithParam(&g_multiAccountsReceiveView, &coin, sizeof(coin));
        break;
    case HOME_WALLET_CARD_ARWEAVE: {
        bool shouldGenerateArweaveXPub = IsArweaveSetupComplete();
        if (!shouldGenerateArweaveXPub) {
            GuiOpenARAddressNoticeWindow();
            break;
        }
        GuiFrameOpenViewWithParam(&g_standardReceiveView, &coin, sizeof(coin));
        break;
    }
    default:
        GuiFrameOpenViewWithParam(&g_standardReceiveView, &coin, sizeof(coin));
        break;
    }
}

void GuiRemoveKeyboardWidget(void)
{
    if (g_keyboardWidget != NULL) {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
    }
    GuiModelRsaGenerateKeyPair();
}

void RecalculateManageWalletState(void)
{
    WalletState_t walletState[HOME_WALLET_CARD_BUTT];
    memcpy(walletState, g_walletState, sizeof(g_walletState));
    AccountPublicHomeCoinGet(g_walletState, NUMBER_OF_ARRAYS(g_walletState));
    AccountPublicHomeCoinSet(walletState, NUMBER_OF_ARRAYS(walletState));
}

void GuiShowRsaInitializatioCompleteHintbox(void)
{
    GuiPendingHintBoxRemove();
    ClearSecretCache();
    GuiCreateInitializatioCompleteHintbox();
}

void GuiHomePasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void ScanQrCodeHandler(lv_event_t *e)
{
    g_isManageClick = false;
    if (g_countDownTimer != NULL) {
        lv_timer_del(g_countDownTimer);
        g_countDownTimer = NULL;
    }

    GuiFrameOpenView(lv_event_get_user_data(e));
}

void ConfirmManageAssetsHandler(lv_event_t *e)
{
    g_currentPage = 0;
    g_coinCurrentPage = 0;
    g_currentFilter = COIN_FILTER_MAIN;
    UpdateManageWalletState(true);
    UpdateHomeConnectWalletCard(GestureNone);
    GUI_DEL_OBJ(g_manageCont)
    GuiHomeRefresh();
}

static void OpenMoreViewHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_moreHintbox)
    GuiFrameOpenView(lv_event_get_user_data(e));
}

static void OpenMoreSettingHandler(lv_event_t *e)
{
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("home_more_connect_wallet"), .src = &imgConnect, .callBack = OpenMoreViewHandler, &g_connectWalletView},
        {.name = _("device_setting_mid_btn"), .src = &imgSettings, .callBack = OpenMoreViewHandler, &g_settingView},
    };
    g_moreHintbox = GuiCreateMoreInfoHintBox(NULL, NULL, moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), true, &g_moreHintbox);
}

static void GuiUpdateCoinListWidget(HomeGesture_t gesture)
{
    int heightIndex = 0;
    uint8_t currentCoinAmount = 0;
    uint8_t totalCoinAmount = 0;

    lv_obj_t *coinListCont = g_coinListCont;
    lv_obj_clean(coinListCont);
    int startIndex = g_currentFilter == COIN_FILTER_MAIN ? HOME_WALLET_CARD_BTC : HOME_WALLET_CARD_TIA;
    int endIndex = g_currentFilter == COIN_FILTER_MAIN ? HOME_WALLET_CARD_TIA : HOME_WALLET_CARD_BUTT;
    for (int i = 0; i < HOME_WALLET_CARD_BUTT; i++) {
        g_walletState[i].checkBox = NULL;
        if (i >= startIndex && i < endIndex && g_walletState[i].enable) {
            totalCoinAmount++;
        }
    }
    
    UpdateCoinStartIndex(gesture, totalCoinAmount);
    lv_obj_t *pageSelectCont = GuiCreateContainerWithParent(coinListCont, 480, 6);
    lv_obj_align(pageSelectCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    GuiDrawPageSelectLed(pageSelectCont, (totalCoinAmount % COIN_PER_PAGE == 0) ? (totalCoinAmount / COIN_PER_PAGE) : (totalCoinAmount / COIN_PER_PAGE + 1), g_coinCurrentPage);
    for (int i = startIndex, j = startIndex; i < endIndex; i++) {
        if (GetIsTempAccount() && g_walletState[i].index == HOME_WALLET_CARD_ARWEAVE || !g_walletState[i].enable) {
            j++;
            continue;
        }

        if ((i - j) < g_coinCurrentPage * COIN_PER_PAGE) {
            continue;
        }
        lv_obj_t *coinLabel = GuiCreateTextLabel(coinListCont, g_coinCardArray[i].coin);
        lv_obj_t *chainLabel = GuiCreateNoticeLabel(coinListCont, g_coinCardArray[i].chain);
        lv_obj_t *icon = GuiCreateImg(coinListCont, g_coinCardArray[i].icon);
        lv_obj_t *checkbox = GuiCreateMultiCheckBox(coinListCont, _(""));
        lv_obj_set_style_pad_top(checkbox, 32, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_size(checkbox, 446, 96);
        g_walletState[i].checkBox = checkbox;
        uint8_t tableLen = 4;
        GuiButton_t table[4] = {
            {.obj = icon, .align = LV_ALIGN_LEFT_MID, .position = {24, 0}},
            {.obj = coinLabel, .align = LV_ALIGN_DEFAULT, .position = {100, 13}},
            {.obj = chainLabel, .align = LV_ALIGN_DEFAULT, .position = {100, 53}},
            {.obj = checkbox, .align = LV_ALIGN_TOP_MID, .position = {-10, 0}},
        };
        lv_obj_clear_flag(checkbox, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t *button = GuiCreateButton(coinListCont, 456, 96, table, tableLen,
                                           NULL, &g_walletState[i]);
        lv_obj_add_event_cb(button, CoinScrollHandler, LV_EVENT_ALL, &g_walletState[i]);

        g_walletButton[i] = button;
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 96 * heightIndex);
        heightIndex++;
        if (currentCoinAmount++ == (COIN_PER_PAGE - 1)) {
            break;
        }
    }

    if (GetMnemonicType() == MNEMONIC_TYPE_TON) {
        lv_obj_t *label = GuiCreateIllustrateLabel(coinListCont, _("import_ton_mnemonic_desc"));
        lv_obj_set_width(label, 416);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 144);
    }
    UpdateManageWalletState(false);
}

static void GuiUpdateCoinManagerHandler(lv_event_t *e)
{
    char *selectFilter = lv_event_get_user_data(e);
    lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));
    if (strcmp(g_coinFilter[g_currentFilter], selectFilter) == 0) {
        return;
    }
    lv_obj_set_style_border_width(lv_obj_get_child(parent, g_currentFilter), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    g_currentFilter = !g_currentFilter;
    g_coinCurrentPage = 0;
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_set_style_border_color(obj, RED_COLOR, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    GuiUpdateCoinListWidget(GestureNone);
}

static void OpenManageAssetsHandler(lv_event_t *e)
{
    if (g_isManageClick == false) {
        return;
    }
    memcpy(&g_walletBakState, &g_walletState, sizeof(g_walletState));

    g_manageCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                      GUI_MAIN_AREA_OFFSET);

    lv_obj_t *filterBar = GuiCreateContainerWithParent(g_manageCont, 408, 62);
    lv_obj_align(filterBar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(filterBar, LV_FLEX_FLOW_ROW);

    for (int i = 0; i < g_coinFilterNum; i++) {
        lv_obj_t *btn = GuiCreateBtnWithFont(filterBar, g_coinFilter[i], &openSansEnIllustrate);
        lv_obj_set_size(btn, 200, 62);
        lv_obj_set_style_radius(btn, 0, 0);
        lv_obj_set_style_bg_color(btn, BLACK_COLOR, 0);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_style_border_color(btn, ORANGE_COLOR, 0);
        lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, i == g_currentFilter ? 2 : 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(btn, GuiUpdateCoinManagerHandler, LV_EVENT_CLICKED, g_coinFilter[i]);
    }

    lv_obj_add_flag(g_manageCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(g_manageCont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(g_manageCont);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 62);

    lv_obj_t *coinListCont = GuiCreateContainerWithParent(g_manageCont, 480, 542 - 62 - 1);
    g_coinListCont = coinListCont;
    lv_obj_clear_flag(coinListCont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(coinListCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(coinListCont, CoinScrollHandler, LV_EVENT_ALL, NULL);
    lv_obj_align(coinListCont, LV_ALIGN_TOP_MID, 0, 62 + 1);
    lv_obj_set_align(coinListCont, LV_ALIGN_DEFAULT);

    lv_obj_t *btn = GuiCreateBtn(g_manageCont, USR_SYMBOL_CHECK);
    lv_obj_add_event_cb(btn, ConfirmManageAssetsHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);

    lv_obj_t *label = GuiCreateTextLabel(g_manageCont, "");
    lv_obj_align_to(label, btn, LV_ALIGN_OUT_LEFT_MID, -300, 0);
    lv_label_set_recolor(label, true);

    g_manageWalletLabel = label;

    GuiUpdateCoinListWidget(GestureNone);

    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("home_manage_assets"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnManageWalletHandler, NULL);
    // TODO: add search
    // GuiNvsBarSetRightCb(NVS_BAR_SEARCH, NULL, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}

void GuiHomeSetWalletDesc(WalletDesc_t *wallet)
{
    GuiNvsBarSetWalletName((const char *)wallet->name);
    GuiSetEmojiIconIndex(wallet->iconIndex);
    SetStatusBarEmojiIndex(wallet->iconIndex);
    GuiNvsBarSetWalletIcon(GuiGetEmojiIconImg());
}

static void HandleDraggingGesture(lv_event_t * e, const ScrollHandler_t *handler)
{
#define SWIPE_THRESHOLD 50
    lv_event_code_t code = lv_event_get_code(e);
    static lv_point_t touchStart;
    static lv_point_t touchEnd;
    static bool isDragging = false;

    if (code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        lv_indev_get_point(indev, &touchStart);
        isDragging = true;
    } else if (code == LV_EVENT_PRESSING) {
        if (isDragging) {
            lv_indev_t * indev = lv_indev_get_act();
            lv_indev_get_point(indev, &touchEnd);
        }
    } else if (code == LV_EVENT_RELEASED) {
        if (isDragging) {
            lv_indev_t * indev = lv_indev_get_act();
            lv_indev_get_point(indev, &touchEnd);
            int16_t diffX = touchEnd.x - touchStart.x;
            int16_t diffY = touchEnd.y - touchStart.y;
            if (abs(diffX) > SWIPE_THRESHOLD) {
                if (diffX < 0) {
                    handler->OnSwipeRight(GestureRight);
                    g_isScrolling = true;
                } else {
                    handler->OnSwipeLeft(GestureLeft);
                    g_isScrolling = true;
                }
            } else if (abs(diffY) > SWIPE_THRESHOLD) {
                // todo
            } else {
                handler->OnClick(e);
            }

            lv_timer_t *timer = lv_timer_create(ResetScrollState, 200, NULL);
            lv_timer_set_repeat_count(timer, 1);
            isDragging = false;
        }
    }
}

static void HomeCoinClick(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    if (obj != g_homeWalletCardCont) {
        HOME_WALLET_CARD_ENUM coin;
        coin = *(HOME_WALLET_CARD_ENUM *)lv_event_get_user_data(e);
        CoinDealHandler(coin);
    }
}

static void HomeScrollHandler(lv_event_t * e)
{
    static const ScrollHandler_t handler = {
        .OnSwipeLeft = UpdateHomeConnectWalletCard,
        .OnSwipeRight = UpdateHomeConnectWalletCard,
        .OnClick = HomeCoinClick
    };
    HandleDraggingGesture(e, &handler);
}

static void CoinManagerClick(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    if (obj != g_coinListCont) {
        WalletState_t *wallet = lv_event_get_user_data(e);
        g_walletBakState[wallet->index].state = !lv_obj_has_state(wallet->checkBox, LV_STATE_CHECKED);
        UpdateManageWalletState(false);
    }
}

static void CoinScrollHandler(lv_event_t * e)
{
    static const ScrollHandler_t handler = {
        .OnSwipeLeft = GuiUpdateCoinListWidget,
        .OnSwipeRight = GuiUpdateCoinListWidget,
        .OnClick = CoinManagerClick
    };
    HandleDraggingGesture(e, &handler);
}

static void ResetScrollState(lv_timer_t * timer)
{
    g_isScrolling = false;
    lv_timer_del(timer);
}

void GuiHomeAreaInit(void)
{
    GuiInitWalletState();
    g_pageWidget = CreatePageWidget();
    g_homeViewCont = g_pageWidget->contentZone;

    lv_obj_t *walletCardCont = GuiCreateContainerWithParent(g_homeViewCont, lv_obj_get_width(lv_scr_act()),
                               lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_event_cb(walletCardCont, HomeScrollHandler, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(walletCardCont, LV_EVENT_CLICKED);
    lv_obj_set_align(walletCardCont, LV_ALIGN_DEFAULT);
    lv_obj_clear_flag(walletCardCont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_set_scrollbar_mode(walletCardCont, LV_SCROLLBAR_MODE_OFF);
    g_homeWalletCardCont = walletCardCont;

    lv_obj_t *img = GuiCreateImg(lv_scr_act(), &imgScan);
    lv_obj_align(img, LV_ALIGN_BOTTOM_RIGHT, -32, -32);
    lv_obj_add_event_cb(img, ScanQrCodeHandler, LV_EVENT_CLICKED, &g_scanView);
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

static void AddFlagCountDownTimerHandler(lv_timer_t *timer)
{
    g_isManageClick = true;
    lv_timer_del(timer);
    g_countDownTimer = NULL;
    UNUSED(g_countDownTimer);
}

void ClearHomePageCurrentIndex(void)
{
    g_currentPage = 0;
    g_coinCurrentPage = 0;
}

void GuiHomeRestart(void)
{
    g_currentPage = 0;
    g_coinCurrentPage = 0;
    g_currentFilter = COIN_FILTER_MAIN;
    GUI_DEL_OBJ(g_manageCont)
    GUI_DEL_OBJ(g_noticeWindow)
    GuiHomeRefresh();
}

void GuiHomeRefresh(void)
{
    static bool isFirstBeta = true;
#ifdef RUST_MEMORY_DEBUG
    PrintRustMemoryStatus();
#endif
    if (GetCurrentAccountIndex() > 2) {
        return;
    }
    GuiInitWalletState();
    g_countDownTimer = lv_timer_create(AddFlagCountDownTimerHandler, 500, NULL);
    GuiSetSetupPhase(SETUP_PAHSE_DONE);
    if (g_manageCont != NULL) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("home_manage_assets"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnManageWalletHandler, NULL);
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
    UpdateHomeConnectWalletCard(GestureNone);
    if (isFirstBeta && SOFTWARE_VERSION_BUILD % 2) {
        CreateBetaNotice();
        isFirstBeta = false;
    }
    CloseArHintbox();
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
    GUI_DEL_OBJ(g_noticeWindow);
}
#endif
