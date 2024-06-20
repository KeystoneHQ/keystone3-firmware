#include "gui_connect_wallet_widgets.h"
#include "account_public_info.h"
#include "gui.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_keyboard.h"
#include "gui_status_bar.h"
#include "gui_views.h"
#include "gui_wallet.h"
#include "rust.h"
#include "user_memory.h"
#ifndef BTC_ONLY
#include "gui_multi_path_coin_receive_widgets.h"
#include "gui_keyboard_hintbox.h"
#include "gui_pending_hintbox.h"
#endif
#include "account_manager.h"
#include "gui_animating_qrcode.h"
#include "gui_global_resources.h"
#include "gui_page.h"
#include "keystore.h"
#ifndef BTC_ONLY
#include "gui_connect_ada_widgets.h"
#endif
#include "gui_select_address_widgets.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

#define DERIVATION_PATH_EG_LEN 2

typedef enum {
    CONNECT_WALLET_SELECT_WALLET = 0,
    CONNECT_WALLET_QRCODE,

    CONNECT_WALLET_BUTT,
} CONNECT_WALLET_ENUM;

WalletListItem_t g_walletListArray[] = {
#ifndef BTC_ONLY
    {WALLET_LIST_OKX, &walletListOkx, true},
    {WALLET_LIST_METAMASK, &walletListMetaMask, true},
    {WALLET_LIST_BLUE, &walletListBlue, true},
    {WALLET_LIST_SPARROW, &walletListSparrow, true},
    {WALLET_LIST_RABBY, &walletListRabby, true},
    {WALLET_LIST_ETERNL, &walletListEternl, true},
    {WALLET_LIST_UNISAT, &walletListUniSat, true},
    // {WALLET_LIST_YOROI, &walletListYoroi, true},
    {WALLET_LIST_TYPHON, &walletListTyphon, true},
    {WALLET_LIST_SAFE, &walletListSafe, true},
    {WALLET_LIST_BLOCK_WALLET, &walletListBlockWallet, true},
    {WALLET_LIST_SOLFARE, &walletListSolfare, true},
    {WALLET_LIST_XRP_TOOLKIT, &walletListXRPToolkit, true},
    {WALLET_LIST_PETRA, &walletListPetra, true},
    {WALLET_LIST_KEPLR, &walletListKeplr, true},
    {WALLET_LIST_ARCONNECT, &walletListArConnect, true},
    {WALLET_LIST_IMTOKEN, &walletListImToken, true},
    {WALLET_LIST_FEWCHA, &walletListFewcha, true},
    {WALLET_LIST_ZAPPER, &walletListZapper, true},
    {WALLET_LIST_YEARN_FINANCE, &walletListYearn, true},
    {WALLET_LIST_SUSHISWAP, &walletListSushi, true},
    {WALLET_LIST_TONKEEPER, &walletListTonkeeper, false},
#else
    {WALLET_LIST_BLUE, &walletListBtcBlue, true, false},
    {WALLET_LIST_SPARROW, &walletListBtcSparrow, true, false},
    {WALLET_LIST_NUNCHUK, &walletListBtcNunchuk, true, false},
// {WALLET_LIST_SPECTER,   &walletListBtcSpecter,  true,   true},
    // {WALLET_LIST_UNISAT,    &walletListBtcUniSat,      true,   true},
#endif
};

typedef struct ConnectWalletWidget {
    uint8_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    WALLET_LIST_INDEX_ENUM walletIndex;
    lv_obj_t *qrCode;
} ConnectWalletWidget_t;

#ifndef BTC_ONLY
typedef struct {
    int8_t index;
    const char *coin;
    const lv_img_dsc_t *icon;
} CoinCard_t;

static const CoinCard_t g_fewchaCoinCardArray[FEWCHA_COINS_BUTT] = {
    {
        .index = APT,
        .coin = "Aptos",
        .icon = &coinApt,
    },
    {
        .index = SUI,
        .coin = "Sui",
        .icon = &coinSui,
    },
};

static const lv_img_dsc_t *g_metaMaskCoinArray[5] = {
    &coinEth, &coinBnb, &coinAva, &coinMatic, &coinScroll,
};

static const lv_img_dsc_t *g_ethWalletCoinArray[4] = {
    &coinEth,
    &coinBnb,
    &coinAva,
    &coinMatic,
};

static const lv_img_dsc_t *g_okxWalletCoinArray[] = {
    &coinBtc, &coinEth, &coinBnb, &coinMatic, &coinOkb,
    &coinTrx, &coinLtc, &coinBch, &coinDash,
};

static const lv_img_dsc_t *g_blueWalletCoinArray[4] = {
    &coinBtc,
};

static const lv_img_dsc_t *g_UniSatCoinArray[5] = {
    &coinBtc, &coinOrdi, &coinSats, &coinMubi, &coinTrac,
};

static const lv_img_dsc_t *g_keplrCoinArray[8] = {
    &coinAtom, &coinOsmo, &coinBld,  &coinAkt,
    &coinXprt, &coinAxl,  &coinBoot, &coinCro,
};

static const lv_img_dsc_t *g_arconnectCoinArray[1] = {
    &coinAr,
};

static const lv_img_dsc_t *g_fewchaCoinArray[FEWCHA_COINS_BUTT] = {
    &coinApt,
    &coinSui,
};

static const lv_img_dsc_t *g_petraCoinArray[1] = {
    &coinApt,
};

static const lv_img_dsc_t *g_solfareCoinArray[1] = {
    &coinSol,
};

static const lv_img_dsc_t *g_tonKeeperCoinArray[1] = {
    &coinTon,
};

static CoinState_t g_defaultFewchaState[FEWCHA_COINS_BUTT] = {
    {APT, true},
    {SUI, false},
};

typedef struct {
    const char *accountType;
    const char *path;
} ChangeDerivationItem_t;

const static ChangeDerivationItem_t g_changeDerivationList[] = {
    {"BIP44 Standard", "#8E8E8E m/44'/60'/0'/0/##F5870A X#"},
    {"Ledger Live", "#8E8E8E m/44'/60'/##F5870A X##8E8E8E '/0/0#"},
    {"Ledger Legacy", "#8E8E8E m/44'/60'/0'/##F5870A X#"},
};

const static ChangeDerivationItem_t g_solChangeDerivationList[] = {
    {"Account-based Path", "#8E8E8E m/44'/501'/##F5870A X##8E8E8E '#"},
    {"Single Account Path", "#8E8E8E m/44'/501'#"},
    {"Sub-account Path", "#8E8E8E m/44'/501'/##F5870A X##8E8E8E '/0'#"},
};

static uint16_t g_chainAddressIndex[3] = {0};
static uint8_t g_currentSelectedPathIndex[3] = {0};
static lv_obj_t *g_coinListCont = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;

#endif

static ConnectWalletWidget_t g_connectWalletTileView;
static PageWidget_t *g_pageWidget;

#ifndef BTC_ONLY
static void UpdategAddress(void);
static void GetEgAddress(void);
static void GetEthEgAddress(void);
static void initFewchaCoinsConfig(void);
#endif
static void OpenQRCodeHandler(lv_event_t *e);
#ifndef BTC_ONLY
static void ReturnShowQRHandler(lv_event_t *e);
static void UpdateFewchaCoinStateHandler(lv_event_t *e);
static void JumpSelectCoinPageHandler(lv_event_t *e);
static void ConfirmSelectFewchaCoinsHandler(lv_event_t *e);
static void GuiCreateSelectFewchaCoinWidget();
#endif
void ConnectWalletReturnHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);
#ifndef BTC_ONLY
static void AddMetaMaskCoins(void);
static void AddOkxWalletCoins(void);
static void AddBlueWalletCoins(void);
static void AddFewchaCoins(void);
static void AddKeplrCoins(void);
static void AddSolflareCoins(void);
static void ShowEgAddressCont(lv_obj_t *egCont);
static uint8_t GetCurrentSelectedIndex();
static bool HasSelectAddressWidget();
#endif

#ifndef BTC_ONLY
CoinState_t g_fewchaCoinState[FEWCHA_COINS_BUTT];
static char g_derivationPathAddr[LedgerLegacy + 1][DERIVATION_PATH_EG_LEN][64];
static char g_solDerivationPathAddr[SOLBip44Change + 1][DERIVATION_PATH_EG_LEN]
[64];

static lv_obj_t *g_derivationCheck[LedgerLegacy + 1];
static ETHAccountType g_currentEthPathIndex[3] = {Bip44Standard, Bip44Standard,
                                                  Bip44Standard
                                                 };
static SOLAccountType g_currentSOLPathIndex[3] = {SOLBip44, SOLBip44, SOLBip44};

static lv_obj_t *g_egAddress[DERIVATION_PATH_EG_LEN];
static lv_obj_t *g_egAddressIndex[DERIVATION_PATH_EG_LEN];

static CoinState_t g_tempFewchaCoinState[FEWCHA_COINS_BUTT];
#endif

static lv_obj_t *g_coinCont = NULL;
static lv_obj_t *g_coinTitleLabel = NULL;
static lv_obj_t *g_openMoreHintBox = NULL;
static lv_obj_t *g_bottomCont = NULL;
static lv_obj_t *g_manageImg = NULL;
static bool g_isCoinReselected = false;
static lv_obj_t *g_derivationPathCont = NULL;
#ifndef BTC_ONLY
static char **g_derivationPathDescs = NULL;
static lv_obj_t *g_derivationPathConfirmBtn = NULL;
static lv_obj_t *g_egCont = NULL;
#endif

static void QRCodePause(bool);

static void GuiInitWalletListArray()
{
    //open all
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        g_walletListArray[i].enable = true;
    }
    if (GetMnemonicType() == MNEMONIC_TYPE_TON) {
        for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
            if (g_walletListArray[i].index == WALLET_LIST_TONKEEPER) {
                g_walletListArray[i].enable = true;
            } else {
                g_walletListArray[i].enable = false;
            }

        }
    } else {
        for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
#ifndef BTC_ONLY
            if (g_walletListArray[i].index == WALLET_LIST_ETERNL ||
                    g_walletListArray[i].index == WALLET_LIST_TYPHON) {
                if (GetMnemonicType() == MNEMONIC_TYPE_SLIP39) {
                    g_walletListArray[i].enable = false;
                } else {
                    g_walletListArray[i].enable = true;
                }
            } else if (g_walletListArray[i].index == WALLET_LIST_ARCONNECT) {
                g_walletListArray[i].enable = !GetIsTempAccount();
            } else if (g_walletListArray[i].index == WALLET_LIST_TONKEEPER) {
                g_walletListArray[i].enable = false;
            }
#else
            if (GetCurrentWalletIndex() != SINGLE_WALLET) {
                if (g_walletListArray[i].index == WALLET_LIST_SPECTER ||
                        g_walletListArray[i].index == WALLET_LIST_UNISAT) {
                    g_walletListArray[i].enable = false;
                } else {
                    g_walletListArray[i].enable = true;
                }

            } else {
                g_walletListArray[i].enable = true;
            }
#endif
        }
    }
}

#ifndef BTC_ONLY
static bool IsEVMChain(int walletIndex)
{
    switch (walletIndex) {
    case WALLET_LIST_METAMASK:
    case WALLET_LIST_RABBY:
    case WALLET_LIST_SAFE:
    case WALLET_LIST_BLOCK_WALLET:
    case WALLET_LIST_ZAPPER:
    case WALLET_LIST_YEARN_FINANCE:
    case WALLET_LIST_SUSHISWAP:
        return true;
    default:
        return false;
    }
}

static bool IsSOL(int walletIndex)
{
    switch (walletIndex) {
    case WALLET_LIST_SOLFARE:
        return true;
    default:
        return false;
    }
}
#endif

static void OpenQRCodeHandler(lv_event_t *e)
{
    WalletListItem_t *wallet = lv_event_get_user_data(e);
    g_connectWalletTileView.walletIndex = wallet->index;
#ifndef BTC_ONLY
    if (IsEVMChain(g_connectWalletTileView.walletIndex)) {
        g_derivationPathDescs = GetDerivationPathDescs(ETH_DERIVATION_PATH_DESC);
    }
    if (IsSOL(g_connectWalletTileView.walletIndex)) {
        g_derivationPathDescs = GetDerivationPathDescs(SOL_DERIVATION_PATH_DESC);
    }
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_ETERNL ||
            g_connectWalletTileView.walletIndex == WALLET_LIST_TYPHON) {
        GuiCreateConnectADAWalletWidget(g_connectWalletTileView.walletIndex);
        return;
    }
    bool skipGenerateArweaveKey = IsArweaveSetupComplete();
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_ARCONNECT && !skipGenerateArweaveKey) {
        GuiCreateAttentionHintbox(SIG_SETUP_RSA_PRIVATE_KEY_CONNECT_CONFIRM);
        return;
    }
#endif
    g_isCoinReselected = false;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
}

void GuiConnectShowRsaSetupasswordHintbox(void)
{
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

#ifndef BTC_ONLY

static void ReturnShowQRHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_coinListCont)
    QRCodePause(false);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                     ConnectWalletReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                      OpenMoreHandler, NULL);
}

static void UpdateFewchaCoinStateHandler(lv_event_t *e)
{

    lv_obj_t *checkBox = lv_event_get_target(e);
    for (int i = 0; i < FEWCHA_COINS_BUTT; i++) {
        g_tempFewchaCoinState[i].state =
            checkBox == g_defaultFewchaState[i].checkBox;
        if (g_tempFewchaCoinState[i].state) {
            lv_obj_add_flag(g_defaultFewchaState[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_defaultFewchaState[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(g_defaultFewchaState[i].uncheckedImg,
                              LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_defaultFewchaState[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void ConfirmSelectFewchaCoinsHandler(lv_event_t *e)
{

    g_isCoinReselected = true;
    memcpy(g_fewchaCoinState, g_tempFewchaCoinState,
           sizeof(g_tempFewchaCoinState));
    GUI_DEL_OBJ(g_coinListCont)
    GuiAnimatingQRCodeDestroyTimer();
    GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                     ConnectWalletReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                      OpenMoreHandler, &g_connectWalletTileView.walletIndex);
}

static void RefreshAddressIndex(uint32_t index)
{
    if (g_chainAddressIndex[GetCurrentAccountIndex()] != index) {
        g_chainAddressIndex[GetCurrentAccountIndex()] = index;
        GuiAnimatingQRCodeDestroyTimer();
        GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
    } else {
        QRCodePause(false);
    }
    g_coinListCont = NULL;
}

static void JumpSelectCoinPageHandler(lv_event_t *e)
{
    if (g_coinListCont != NULL) {
        return;
    }
#ifndef COMPILE_SIMULATOR
    QRCodePause(true);
#endif
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_FEWCHA) {
        GuiCreateSelectFewchaCoinWidget();
    } else if (HasSelectAddressWidget()) {
        if (g_connectWalletTileView.walletIndex == WALLET_LIST_XRP_TOOLKIT) {
            g_coinListCont = GuiCreateSelectAddressWidget(
                                 CHAIN_XRP, g_chainAddressIndex[GetCurrentAccountIndex()],
                                 RefreshAddressIndex);
        }
    }
}

static void GuiCreateSelectFewchaCoinWidget()
{
    // root container
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()),
                                        lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    g_coinListCont = cont;
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);

    // coin list container
    lv_obj_t *coinListCont = GuiCreateContainerWithParent(cont, 408, 542);
    lv_obj_set_align(coinListCont, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(coinListCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(coinListCont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *labelHint = GuiCreateIllustrateLabel(
                              coinListCont, _("connect_wallet_select_network_hint"));
    lv_obj_set_style_text_opa(labelHint, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *coinLabel;
    lv_obj_t *icon;

    for (int i = 0; i < FEWCHA_COINS_BUTT; i++) {
        coinLabel = GuiCreateTextLabel(coinListCont, g_fewchaCoinCardArray[i].coin);
        icon = GuiCreateScaleImg(coinListCont, g_fewchaCoinCardArray[i].icon, 118);
        g_defaultFewchaState[i].uncheckedImg =
            GuiCreateImg(coinListCont, &imgUncheckCircle);
        g_defaultFewchaState[i].checkedImg =
            GuiCreateImg(coinListCont, &imgMessageSelect);

        if (g_tempFewchaCoinState[i].state) {
            lv_obj_add_flag(g_defaultFewchaState[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_defaultFewchaState[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(g_defaultFewchaState[i].uncheckedImg,
                              LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_defaultFewchaState[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        }

        GuiButton_t table[4] = {
            {
                .obj = coinLabel,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 16},
            },
            {
                .obj = icon,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 60},
            },
            {
                .obj = g_defaultFewchaState[i].checkedImg,
                .align = LV_ALIGN_CENTER,
                .position = {160, 0},
            },
            {
                .obj = g_defaultFewchaState[i].uncheckedImg,
                .align = LV_ALIGN_CENTER,
                .position = {160, 0},
            },
        };
        int buttonNum = 4;
        lv_obj_t *button = GuiCreateButton(
                               coinListCont, lv_obj_get_width(coinListCont), 100, table, buttonNum,
                               UpdateFewchaCoinStateHandler, &g_fewchaCoinState[i]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 0,
                     100 * i + 16 * i + lv_obj_get_height(labelHint) + 24);
        g_defaultFewchaState[i].checkBox = button;
    }

    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_CHECK);
    lv_obj_add_event_cb(btn, ConfirmSelectFewchaCoinsHandler, LV_EVENT_CLICKED,
                        NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);

    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL,
                   _("connect_wallet_select_network"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                     ReturnShowQRHandler, cont);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL,
                      NULL);
}
#endif

static void GuiCreateSelectWalletWidget(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
#ifndef BTC_ONLY
    bool isTon = GetMnemonicType() == MNEMONIC_TYPE_TON;
    if (isTon) {
        WalletListItem_t *t = NULL;
        for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
            if (g_walletListArray[i].index == WALLET_LIST_TONKEEPER) {
                t = &g_walletListArray[i];
                break;
            }
        }
        ASSERT(t != NULL);
        lv_obj_t *img = GuiCreateImg(parent, t->img);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, OpenQRCodeHandler, LV_EVENT_CLICKED,
                            t);
    } else {
        lv_obj_t *img = GuiCreateImg(parent, g_walletListArray[0].img);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, OpenQRCodeHandler, LV_EVENT_CLICKED,
                            &g_walletListArray[0]);
        for (int i = 1, j = 1; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
            if (!g_walletListArray[i].enable) {
                continue;
            }
            lv_obj_t *img = GuiCreateImg(parent, g_walletListArray[i].img);
            lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 136 + (j - 1) * 107);
            lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(img, OpenQRCodeHandler, LV_EVENT_CLICKED,
                                &g_walletListArray[i]);
            j++;
        }
    }
#else
    lv_obj_t *img, *line, *alphaImg;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    line = GuiCreateLine(parent, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_t *baseView = NULL;
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        if (!g_walletListArray[i].enable) {
            continue;
        }
        // temporary fix, when the multi-signature testnet is opened, the logic here
        // needs to be rewritten
        if (GetCurrentWalletIndex() == SINGLE_WALLET && GetIsTestNet() &&
                g_walletListArray[i].index == WALLET_LIST_BLUE) {
            continue;
        }
        img = GuiCreateImg(parent, g_walletListArray[i].img);
        if (baseView == NULL) {
            lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 9);
        } else {
            lv_obj_align_to(img, baseView, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        }
        baseView = img;
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, OpenQRCodeHandler, LV_EVENT_CLICKED,
                            &g_walletListArray[i]);
        if (g_walletListArray[i].alpha) {
            alphaImg = GuiCreateImg(img, &imgAlpha);
            lv_obj_align(alphaImg, LV_ALIGN_RIGHT_MID, -219, 0);
        }
        line = GuiCreateLine(parent, points, 2);
        lv_obj_align_to(line, baseView, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }
#endif
}

#ifndef BTC_ONLY
static void GuiCreateSupportedNetworks(uint8_t index)
{
    if (g_coinCont != NULL && g_manageImg != NULL && g_coinTitleLabel != NULL) {
        return;
    }
    lv_obj_clean(g_bottomCont);
    if (index == WALLET_LIST_UNISAT) {
        g_coinTitleLabel = GuiCreateNoticeLabel(g_bottomCont, _("connect_wallet_supported_tokens"));
    } else {
        g_coinTitleLabel = GuiCreateNoticeLabel(g_bottomCont, _("connect_wallet_supported_networks"));
    }
    lv_obj_align(g_coinTitleLabel, LV_ALIGN_TOP_LEFT, 36, 12);
    lv_obj_add_event_cb(g_bottomCont, JumpSelectCoinPageHandler, LV_EVENT_CLICKED, NULL);
    g_coinCont = GuiCreateContainerWithParent(g_bottomCont, 280, 30);
    lv_obj_align(g_coinCont, LV_ALIGN_TOP_LEFT, 36, 50);
    lv_obj_set_style_bg_color(g_coinCont, DARK_BG_COLOR, LV_PART_MAIN);

    g_manageImg = GuiCreateImg(g_bottomCont, &imgManage);
    lv_obj_set_style_img_opa(g_manageImg, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(g_manageImg, LV_ALIGN_BOTTOM_RIGHT, -45, -41);
    lv_obj_add_flag(g_manageImg, LV_OBJ_FLAG_HIDDEN);
}
#endif

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_scan"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
#ifndef BTC_ONLY
    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 482);
#else
    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 408);
#endif
    lv_obj_add_flag(qrCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align_to(qrCont, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(qrCont, 336, 336);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_color(qrBgCont, WHITE_COLOR, LV_PART_MAIN);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(qrBgCont, 294, 294);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);
    g_connectWalletTileView.qrCode = qrcode;

    g_bottomCont = GuiCreateContainerWithParent(qrCont, 408, 104);
    lv_obj_align(g_bottomCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(g_bottomCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_bottomCont, LV_OPA_0,
                            LV_STATE_DEFAULT | LV_PART_MAIN);
#ifndef BTC_ONLY
    // GuiCreateSupportedNetworks(g_connectWalletTileView.walletIndex);
#else
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        lv_obj_t *button = GuiCreateImgLabelAdaptButton(
                               parent, _("multisig_connect_wallet_notice"), &imgTwoSmallKey, UnHandler,
                               NULL);
        lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -24);
        lv_obj_set_style_text_opa(lv_obj_get_child(button, 1), LV_OPA_80,
                                  LV_PART_MAIN);
    }
#endif
}

#ifndef BTC_ONLY
static void AddMetaMaskCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 5; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_metaMaskCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 132, 2);
}

static void AddEthWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 4; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_ethWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 132, 2);
}

static void AddOkxWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0;
            i < sizeof(g_okxWalletCoinArray) / sizeof(g_okxWalletCoinArray[0]);
            i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_okxWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 132, 2);
}

static void AddBlueWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 1; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_blueWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddUniSatWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 5; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_UniSatCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
    // Add more
    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * 5, 2);
}

static void AddKeplrCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 8; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_keplrCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 256, 2);
}

static void AddArConnectCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, g_arconnectCoinArray[0]);
    lv_img_set_zoom(img, 110);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);
}

static void AddFewchaCoins()
{
    lv_obj_add_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_manageImg, LV_OBJ_FLAG_HIDDEN);
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    int count = 0;
    for (int i = 0; i < FEWCHA_COINS_BUTT; i++) {
        if (g_fewchaCoinState[i].state) {
            lv_obj_t *img = GuiCreateImg(
                                g_coinCont, g_fewchaCoinArray[g_fewchaCoinState[i].index]);
            lv_img_set_zoom(img, 110);
            lv_img_set_pivot(img, 0, 0);
            lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * count, 0);
            count++;
        }
    }
}

static void AddPetraCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 1; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_petraCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddTonCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 1; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_tonKeeperCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddChainAddress(void)
{
    if (lv_obj_get_child_cnt(g_bottomCont) > 0) {
        lv_obj_clean(g_bottomCont);
        g_manageImg = NULL;
        g_coinCont = NULL;
    }
    lv_obj_add_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);

    char name[BUFFER_SIZE_32] = {0};
    snprintf_s(name, sizeof(name), "%s-%d", _("account_head"),
               g_chainAddressIndex[GetCurrentAccountIndex()] + 1);
    lv_obj_t *label = GuiCreateIllustrateLabel(g_bottomCont, name);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 24);

    char addr[BUFFER_SIZE_256] = {0};
    CutAndFormatString(
        addr, sizeof(addr),
        GuiGetXrpAddressByIndex(g_chainAddressIndex[GetCurrentAccountIndex()]),
        20);
    label = GuiCreateNoticeLabel(g_bottomCont, addr);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 58);

    label = GuiCreateImg(g_bottomCont, &imgArrowRight);
    lv_obj_align(label, LV_ALIGN_CENTER, 150, 0);
}

static void AddSolflareCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 1; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_solfareCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}
#endif

void GuiConnectWalletInit(void)
{
    GuiInitWalletListArray();
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, CONNECT_WALLET_SELECT_WALLET,
                                          0, LV_DIR_HOR);
    GuiCreateSelectWalletWidget(tile);

    tile = lv_tileview_add_tile(tileView, CONNECT_WALLET_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(tile);

    g_connectWalletTileView.currentTile = CONNECT_WALLET_SELECT_WALLET;
    g_connectWalletTileView.tileView = tileView;
    g_connectWalletTileView.cont = cont;

    lv_obj_set_tile_id(g_connectWalletTileView.tileView,
                       g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
}

#ifndef BTC_ONLY
UREncodeResult *GuiGetFewchaData(void)
{
    GuiChainCoinType coin = CHAIN_APT;
    if (g_fewchaCoinState[1].state) {
        coin = CHAIN_SUI;
    }
    return GuiGetFewchaDataByCoin(coin);
}

UREncodeResult *GuiGetXrpToolkitData(void)
{
    return GuiGetXrpToolkitDataByIndex(
               g_chainAddressIndex[GetCurrentAccountIndex()]);
}

UREncodeResult *GuiGetADAData(void)
{
    return GuiGetADADataByIndex(g_chainAddressIndex[GetCurrentAccountIndex()]);
}

UREncodeResult *GuiGetTonData(void)
{
    bool isTon = GetMnemonicType() == MNEMONIC_TYPE_TON;
    if (isTon) {
        char* xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TON_NATIVE);
        char* walletName = GetWalletName();
        if (walletName == NULL) {
            walletName = "Keystone";
        }
        return get_tonkeeper_wallet_ur(xpub, walletName, NULL, 0, NULL);
    } else {
        ASSERT(false);
    }
}

void GuiPrepareArConnectWalletView(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
}

void GuiSetupArConnectWallet(void)
{
    RsaGenerateKeyPair(false);
}

#endif

void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index)
{
    GenerateUR func = NULL;
    SetWallet(g_pageWidget->navBarWidget, index, NULL);
#ifndef BTC_ONLY
    GuiCreateSupportedNetworks(index);
    lv_label_set_text(g_coinTitleLabel, _("connect_wallet_supported_networks"));
    lv_obj_clear_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(g_manageImg, LV_OBJ_FLAG_HIDDEN);
#endif
    switch (index) {
#ifndef BTC_ONLY
    case WALLET_LIST_METAMASK:
        func = GuiGetMetamaskData;
        AddMetaMaskCoins();
        break;
    case WALLET_LIST_RABBY:
    case WALLET_LIST_SAFE:
    case WALLET_LIST_BLOCK_WALLET:
    case WALLET_LIST_ZAPPER:
    case WALLET_LIST_YEARN_FINANCE:
    case WALLET_LIST_SUSHISWAP:
        func = GuiGetMetamaskData;
        AddEthWalletCoins();
        break;
    case WALLET_LIST_IMTOKEN:
        func = GuiGetImTokenData;
        AddEthWalletCoins();
        break;
    case WALLET_LIST_OKX:
        func = GuiGetOkxWalletData;
        AddOkxWalletCoins();
        break;
    case WALLET_LIST_BLUE:
        func = GuiGetBlueWalletBtcData;
        AddBlueWalletCoins();
        break;
    case WALLET_LIST_SPARROW:
        func = GuiGetSparrowWalletBtcData;
        AddBlueWalletCoins();
        break;
    case WALLET_LIST_UNISAT:
        func = GuiGetSparrowWalletBtcData;
        AddUniSatWalletCoins();
        lv_label_set_text(g_coinTitleLabel, _("connect_wallet_supported_tokens"));
        break;
    case WALLET_LIST_SUB:
        break;
    case WALLET_LIST_SOLFARE:
        func = GuiGetSolflareData;
        AddSolflareCoins();
        break;
    case WALLET_LIST_KEPLR:
        func = GuiGetKeplrData;
        AddKeplrCoins();
        break;
    case WALLET_LIST_ARCONNECT:
        func = GuiGetArConnectData;
        AddArConnectCoins();
        break;
    case WALLET_LIST_TYPHON:
        func = GuiGetADAData;
        AddChainAddress();
        break;
    case WALLET_LIST_FEWCHA:
        if (!g_isCoinReselected) {
            initFewchaCoinsConfig();
        }
        func = GuiGetFewchaData;
        AddFewchaCoins();
        break;
    case WALLET_LIST_PETRA:
        func = GuiGetPetraData;
        AddPetraCoins();
        break;
    case WALLET_LIST_XRP_TOOLKIT:
        func = GuiGetXrpToolkitData;
        AddChainAddress();
        break;
    case WALLET_LIST_TONKEEPER:
        func = GuiGetTonData;
        AddTonCoins();
        break;
#else
    case WALLET_LIST_BLUE:
    case WALLET_LIST_NUNCHUK:
        // 84 49 44
        func = GuiGetBlueWalletBtcData;
        break;
    case WALLET_LIST_SPARROW:
        // 84 49 44 86
        func = GuiGetSparrowWalletBtcData;
        break;
    case WALLET_LIST_SPECTER:
        // 84 49
        func = GuiGetSpecterWalletBtcData;
        break;
    case WALLET_LIST_UNISAT:
        func = GuiGetSparrowWalletBtcData;
        break;
#endif
    default:
        return;
    }
    if (func) {
        bool skipGenerateArweaveKey = IsArweaveSetupComplete();
        if (index == WALLET_LIST_ARCONNECT && !skipGenerateArweaveKey) {
            GuiAnimatingQRCodeInitWithLoadingParams(g_connectWalletTileView.qrCode, func, true, _("InitializingRsaTitle"), _("FindingRsaPrimes"));
        } else {
            GuiAnimatingQRCodeInit(g_connectWalletTileView.qrCode, func, true);
        }
    }
}

static void QRCodePause(bool pause)
{
    GuiAnimatingQRCodeControl(pause);
}

void GuiConnectWalletHandleURGenerate(char *data, uint16_t len)
{
    GuiAnimantingQRCodeFirstUpdate(data, len);
}

void GuiConnectWalletHandleURUpdate(char *data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
}

void ConnectWalletReturnHandler(lv_event_t *e)
{
    // CloseQRTimer();
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
}

#ifndef BTC_ONLY
static void initFewchaCoinsConfig(void)
{
    memcpy(g_tempFewchaCoinState, g_defaultFewchaState,
           sizeof(g_defaultFewchaState));
    memcpy(g_fewchaCoinState, g_defaultFewchaState, sizeof(g_defaultFewchaState));
}

ETHAccountType GetMetamaskAccountType(void)
{
    return g_currentEthPathIndex[GetCurrentAccountIndex()];
}

SOLAccountType GetSolflareAccountType(void)
{
    return g_currentSOLPathIndex[GetCurrentAccountIndex()];
}

static int GetAccountType(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        return GetSolflareAccountType();
    default:
        return GetMetamaskAccountType();
    }
}

static void SetAccountType(uint8_t index)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        g_currentSOLPathIndex[GetCurrentAccountIndex()] = index;
        break;
    default:
        g_currentEthPathIndex[GetCurrentAccountIndex()] = index;
        break;
    }
}

static bool IsSelectChanged(void)
{
    return GetCurrentSelectedIndex() != GetAccountType();
}

static bool HasSelectAddressWidget()
{
    return g_connectWalletTileView.walletIndex == WALLET_LIST_XRP_TOOLKIT;
}

static void CloseDerivationHandler(lv_event_t *e)
{
    QRCodePause(false);
    GUI_DEL_OBJ(g_derivationPathCont);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler,
                     NULL);
    SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex,
              NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                      OpenMoreHandler, &g_connectWalletTileView.walletIndex);
}

static void ConfirmDerivationHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED && IsSelectChanged()) {
        SetAccountType(GetCurrentSelectedIndex());
        GuiAnimatingQRCodeDestroyTimer();
        GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
        GUI_DEL_OBJ(g_derivationPathCont);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler,
                         NULL);
        SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex,
                  NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                          OpenMoreHandler, &g_connectWalletTileView.walletIndex);
    }
}

static void UpdateConfirmBtn(void)
{
    if (IsSelectChanged()) {
        lv_obj_set_style_bg_opa(g_derivationPathConfirmBtn, LV_OPA_COVER,
                                LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_derivationPathConfirmBtn, 0),
                                  LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_derivationPathConfirmBtn, LV_OPA_30,
                                LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_derivationPathConfirmBtn, 0),
                                  LV_OPA_30, LV_PART_MAIN);
    }
}

static void GetEthEgAddress(void)
{
    SimpleResponse_c_char *result;
    result = eth_get_address(
                 "44'/60'/0'/0/0",
                 GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD), "44'/60'/0'");
    CutAndFormatString(g_derivationPathAddr[Bip44Standard][0], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result = eth_get_address(
                 "44'/60'/0'/0/1",
                 GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD), "44'/60'/0'");
    CutAndFormatString(g_derivationPathAddr[Bip44Standard][1], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result = eth_get_address(
                 "44'/60'/0'/0/0", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LIVE_0),
                 "44'/60'/0'");
    CutAndFormatString(g_derivationPathAddr[LedgerLive][0], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result = eth_get_address(
                 "44'/60'/1'/0/0", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LIVE_1),
                 "44'/60'/1'");
    CutAndFormatString(g_derivationPathAddr[LedgerLive][1], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result = eth_get_address(
                 "44'/60'/0'/0", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY),
                 "44'/60'/0'");
    CutAndFormatString(g_derivationPathAddr[LedgerLegacy][0], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result = eth_get_address(
                 "44'/60'/0'/1", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY),
                 "44'/60'/0'");
    CutAndFormatString(g_derivationPathAddr[LedgerLegacy][1], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);
}

static void GetSolEgAddress(void)
{
    SimpleResponse_c_char *result;
    result =
        solana_get_address(GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_0));
    CutAndFormatString(g_solDerivationPathAddr[SOLBip44][0], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result =
        solana_get_address(GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_1));
    CutAndFormatString(g_solDerivationPathAddr[SOLBip44][1], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result =
        solana_get_address(GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_ROOT));
    CutAndFormatString(g_solDerivationPathAddr[SOLBip44ROOT][0], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result = solana_get_address(
                 GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_CHANGE_0));
    CutAndFormatString(g_solDerivationPathAddr[SOLBip44Change][0], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);

    result = solana_get_address(
                 GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_CHANGE_1));
    CutAndFormatString(g_solDerivationPathAddr[SOLBip44Change][1], BUFFER_SIZE_64,
                       result->data, 24);
    free_simple_response_c_char(result);
}

static void GetEgAddress(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        GetSolEgAddress();
        break;
    default:
        GetEthEgAddress();
        break;
    }
}

static void UpdateEthEgAddress(uint8_t index)
{
    lv_label_set_text(g_egAddress[0],
                      (const char *)g_derivationPathAddr[index][0]);
    lv_label_set_text(g_egAddress[1],
                      (const char *)g_derivationPathAddr[index][1]);
}

static void UpdateSolEgAddress(uint8_t index)
{
    lv_label_set_text(g_egAddress[0],
                      (const char *)g_solDerivationPathAddr[index][0]);
    if (index != SOLBip44ROOT) {
        lv_label_set_text(g_egAddress[1],
                          (const char *)g_solDerivationPathAddr[index][1]);
    }
}

static void UpdategAddress(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        UpdateSolEgAddress(GetCurrentSelectedIndex());
        break;
    default:
        UpdateEthEgAddress(GetCurrentSelectedIndex());
        break;
    }
}

static void SetCurrentSelectedIndex(uint8_t index)
{
    g_currentSelectedPathIndex[GetCurrentAccountIndex()] = index;
}

static uint8_t GetCurrentSelectedIndex()
{
    return g_currentSelectedPathIndex[GetCurrentAccountIndex()];
}

static void SelectDerivationHandler(lv_event_t *e)
{
    lv_obj_t *newCheckBox = lv_event_get_user_data(e);
    for (int i = 0; i < 3; i++) {
        if (newCheckBox == g_derivationCheck[i]) {
            lv_obj_add_state(newCheckBox, LV_STATE_CHECKED);
            SetCurrentSelectedIndex(i);
            ShowEgAddressCont(g_egCont);
            UpdateConfirmBtn();
        } else {
            lv_obj_clear_state(g_derivationCheck[i], LV_STATE_CHECKED);
        }
    }
}
#endif

static void OpenTutorialHandler(lv_event_t *e)
{
    QRCodePause(true);

    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(wallet));
    GUI_DEL_OBJ(g_openMoreHintBox);
}

#ifndef BTC_ONLY
static const char *GetDerivationPathSelectDes(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        return _("derivation_path_select_sol");
    default:
        return _("derivation_path_select_eth");
    }
}

static const char *GetChangeDerivationAccountType(int i)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        if (i == 0) {
            return _("receive_sol_more_t_base_path");
        } else if (i == 1) {
            return _("receive_sol_more_t_single_path");
        } else if (i == 2) {
            return _("receive_sol_more_t_sub_path");
        }
    default:
        return g_changeDerivationList[i].accountType;
    }
}

static const char *GetChangeDerivationPath(int i)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        return g_solChangeDerivationList[i].path;
    default:
        return g_changeDerivationList[i].path;
    }
}

static char *GetChangeDerivationPathDesc(void)
{
    return g_derivationPathDescs
           [g_currentSelectedPathIndex[GetCurrentAccountIndex()]];
}

static void ShowEgAddressCont(lv_obj_t *egCont)
{
    if (egCont == NULL) {
        printf("egCont is NULL, cannot show eg address\n");
        return;
    }
    lv_obj_clean(egCont);

    lv_obj_t *prevLabel = NULL, *label;
    int egContHeight = 12;
    char *desc = GetChangeDerivationPathDesc();
    if (desc != NULL && strnlen_s(desc, BUFFER_SIZE_128) > 0) {
        label = GuiCreateNoticeLabel(egCont, desc);
        lv_obj_set_width(label, 360);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
        lv_obj_update_layout(label);
        egContHeight += lv_obj_get_height(label);
        prevLabel = label;
    }

    label = GuiCreateNoticeLabel(egCont, _("derivation_path_address_eg"));
    if (prevLabel != NULL) {
        lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    } else {
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    }
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(label);
    egContHeight = egContHeight + 4 + lv_obj_get_height(label);
    prevLabel = label;

    lv_obj_t *index = GuiCreateNoticeLabel(egCont, _("0"));
    lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(index);
    egContHeight = egContHeight + 4 + lv_obj_get_height(index);
    g_egAddressIndex[0] = index;
    prevLabel = index;

    label = GuiCreateIllustrateLabel(egCont, "");
    lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    g_egAddress[0] = label;

    if (!(g_connectWalletTileView.walletIndex == WALLET_LIST_SOLFARE &&
            GetCurrentSelectedIndex() == SOLBip44ROOT)) {
        index = GuiCreateNoticeLabel(egCont, _("1"));
        lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(index);
        egContHeight = egContHeight + 4 + lv_obj_get_height(index);
        g_egAddressIndex[1] = index;
        prevLabel = index;

        label = GuiCreateIllustrateLabel(egCont, "");
        lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
        g_egAddress[1] = label;
    }
    egContHeight += 12;
    lv_obj_set_height(egCont, egContHeight);
    GetEgAddress();
    UpdategAddress();
}

static void OpenDerivationPath()
{
    SetCurrentSelectedIndex(GetAccountType());

    lv_obj_t *bgCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()),
                                          lv_obj_get_height(lv_scr_act()) -
                                          GUI_MAIN_AREA_OFFSET);

    lv_obj_align(bgCont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);

    lv_obj_t *scrollCont = GuiCreateContainerWithParent(
                               bgCont, lv_obj_get_width(lv_scr_act()),
                               lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET - 114);
    lv_obj_align(scrollCont, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label =
        GuiCreateNoticeLabel(scrollCont, GetDerivationPathSelectDes());
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *cont = GuiCreateContainerWithParent(scrollCont, 408, 308);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    for (int i = 0; i < 3; i++) {
        lv_obj_t *accountType =
            GuiCreateTextLabel(cont, GetChangeDerivationAccountType(i));
        lv_obj_t *path = GuiCreateIllustrateLabel(cont, GetChangeDerivationPath(i));
        lv_label_set_recolor(path, true);
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(cont, _(""));
        lv_obj_set_size(checkBox, 45, 45);
        g_derivationCheck[i] = checkBox;
        if (i == GetCurrentSelectedIndex()) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        GuiButton_t table[] = {
            {
                .obj = accountType,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 16},
            },
            {
                .obj = path,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 56},
            },
            {
                .obj = checkBox,
                .align = LV_ALIGN_RIGHT_MID,
                .position = {-24, 0},
            },
        };
        lv_obj_t *button =
            GuiCreateButton(cont, 408, 102, table, NUMBER_OF_ARRAYS(table),
                            SelectDerivationHandler, g_derivationCheck[i]);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, i * 102);
        if (i != 0) {
            static lv_point_t points[2] = {{0, 0}, {360, 0}};
            lv_obj_t *line = (lv_obj_t *)GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, i * 102);
        }
    }

    lv_obj_t *egCont = GuiCreateContainerWithParent(scrollCont, 408, 186);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);
    g_egCont = egCont;
    ShowEgAddressCont(g_egCont);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL,
                   _("derivation_path_change"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                     CloseDerivationHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL,
                      NULL);
    GUI_DEL_OBJ(g_openMoreHintBox);

    lv_obj_t *tmCont = GuiCreateContainerWithParent(bgCont, 480, 114);
    lv_obj_align(tmCont, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(tmCont, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_t *btn = GuiCreateBtn(tmCont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -36, 0);
    lv_obj_add_event_cb(btn, ConfirmDerivationHandler, LV_EVENT_CLICKED, NULL);
    g_derivationPathConfirmBtn = btn;
    UpdateConfirmBtn();

    g_derivationPathCont = bgCont;
}

static void ChangeDerivationPathHandler(lv_event_t *e)
{

    OpenDerivationPath();
    QRCodePause(true);
}
#endif

static void OpenMoreHandler(lv_event_t *e)
{
    int hintboxHeight = 132;
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
#ifndef BTC_ONLY
    if (IsEVMChain(*wallet) || IsSOL(*wallet)) {
        hintboxHeight = 228;
    }
#endif
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0),
                        CloseHintBoxHandler, LV_EVENT_CLICKED,
                        &g_openMoreHintBox);
    lv_obj_t *btn =
        GuiCreateSelectButton(g_openMoreHintBox, _("Tutorial"), &imgTutorial,
                              OpenTutorialHandler, wallet, true);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
#ifndef BTC_ONLY
    if (IsEVMChain(*wallet) || IsSOL(*wallet)) {
        btn = GuiCreateSelectButton(g_openMoreHintBox, _("derivation_path_change"),
                                    &imgPath, ChangeDerivationPathHandler, wallet,
                                    true);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -120);
    }
#endif
}

int8_t GuiConnectWalletNextTile(void)
{
    switch (g_connectWalletTileView.currentTile) {
    case CONNECT_WALLET_SELECT_WALLET:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                         ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                          OpenMoreHandler, &g_connectWalletTileView.walletIndex);
        GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
        break;
    case CONNECT_WALLET_QRCODE:
        return 0;
    }

    g_connectWalletTileView.currentTile++;
    lv_obj_set_tile_id(g_connectWalletTileView.tileView,
                       g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiConnectWalletPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

int8_t GuiConnectWalletPrevTile(void)
{
    switch (g_connectWalletTileView.currentTile) {
    case CONNECT_WALLET_SELECT_WALLET:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler,
                         NULL);
        break;
    case CONNECT_WALLET_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE,
                         CloseTimerCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL,
                       _("connect_wallet_choose_wallet"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL,
                          NULL);
        GuiAnimatingQRCodeDestroyTimer();
        break;
    }
    g_connectWalletTileView.currentTile--;
    lv_obj_set_tile_id(g_connectWalletTileView.tileView,
                       g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiConnectWalletRefresh(void)
{
    GuiCloseAttentionHintbox();
    switch (g_connectWalletTileView.currentTile) {
    case CONNECT_WALLET_SELECT_WALLET:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE,
                         CloseTimerCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL,
                       _("connect_wallet_choose_wallet"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL,
                          NULL);
        break;
    case CONNECT_WALLET_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                         ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                          OpenMoreHandler, &g_connectWalletTileView.walletIndex);
        SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex,
                  NULL);
#ifndef BTC_ONLY
        if (g_coinListCont != NULL) {
            if (g_connectWalletTileView.walletIndex == WALLET_LIST_FEWCHA) {
                GUI_DEL_OBJ(g_coinListCont)
                GuiCreateSelectFewchaCoinWidget();
            } else if (HasSelectAddressWidget()) {
                GuiDestroySelectAddressWidget();
                if (g_connectWalletTileView.walletIndex == WALLET_LIST_XRP_TOOLKIT) {
                    g_coinListCont = GuiCreateSelectAddressWidget(
                                         CHAIN_XRP, g_chainAddressIndex[GetCurrentAccountIndex()],
                                         RefreshAddressIndex);
                }
            }
        }
#endif
        QRCodePause(false);
    }
#ifndef BTC_ONLY
    if (ConnectADAWidgetExist()) {
        CleanConnectADAWidget();
        GuiCreateConnectADAWalletWidget(g_connectWalletTileView.walletIndex);
    }
    if (g_derivationPathCont != NULL) {
        GUI_DEL_OBJ(g_derivationPathCont);
        OpenDerivationPath();
    }
#endif
}

void GuiConnectWalletDeInit(void)
{
    GUI_DEL_OBJ(g_openMoreHintBox)
    GUI_DEL_OBJ(g_manageImg);
    GUI_DEL_OBJ(g_coinCont)
    GUI_DEL_OBJ(g_derivationPathCont)
#ifndef BTC_ONLY
    if (g_coinListCont != NULL && HasSelectAddressWidget()) {
        g_coinListCont = NULL;
        GuiDestroySelectAddressWidget();
    } else {
        GUI_DEL_OBJ(g_coinListCont)
    }
    CleanConnectADAWidget();
#endif
    CloseToTargetTileView(g_connectWalletTileView.currentTile,
                          CONNECT_WALLET_SELECT_WALLET);
    GUI_DEL_OBJ(g_connectWalletTileView.cont)
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

uint8_t GuiConnectWalletGetWalletIndex(void)
{
    return g_connectWalletTileView.walletIndex;
}