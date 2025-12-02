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
#ifdef WEB3_VERSION
#include "gui_multi_path_coin_receive_widgets.h"
#include "gui_connect_ada_widgets.h"
#include "gui_keyboard_hintbox.h"
#include "gui_pending_hintbox.h"
#endif
#include "account_manager.h"
#include "gui_animating_qrcode.h"
#include "gui_global_resources.h"
#include "gui_page.h"
#include "keystore.h"
#include "gui_select_address_widgets.h"
#include "account_public_info.h"

#define DERIVATION_PATH_EG_LEN 2

typedef enum {
    CONNECT_WALLET_SELECT_WALLET = 0,
    CONNECT_WALLET_QRCODE,

    CONNECT_WALLET_BUTT,
} CONNECT_WALLET_ENUM;

typedef struct {
    int8_t                  index;
    bool                    state;
    lv_obj_t                *checkBox;
    lv_obj_t                *uncheckedImg;
    lv_obj_t                *checkedImg;
} CoinState_t;

typedef struct ConnectWalletWidget {
    uint32_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    WALLET_LIST_INDEX_ENUM walletIndex;
    lv_obj_t *qrCode;
} ConnectWalletWidget_t;

WalletListItem_t g_walletListArray[] = {
    {WALLET_LIST_KEYSTONE, &walletListKeystone, false, WALLET_FILTER_BTC | WALLET_FILTER_ETH | WALLET_FILTER_OTHER},
    {WALLET_LIST_OKX, &walletListOkx, true, WALLET_FILTER_BTC | WALLET_FILTER_ETH | WALLET_FILTER_OTHER},
    {WALLET_LIST_METAMASK, &walletListMetaMask, true, WALLET_FILTER_ETH},
    {WALLET_LIST_XAMAN, &walletListXaman, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_BACKPACK, &walletListBackpack, true, WALLET_FILTER_ETH | WALLET_FILTER_SOL | WALLET_FILTER_OTHER},
    {WALLET_LIST_SOLFARE, &walletListSolfare, true, WALLET_FILTER_SOL},
    {WALLET_LIST_NUFI, &walletListNufi, true, WALLET_FILTER_BTC | WALLET_FILTER_ETH | WALLET_FILTER_SOL | WALLET_FILTER_ADA},
    {WALLET_LIST_CORE, &walletListCore, true, WALLET_FILTER_BTC | WALLET_FILTER_ETH | WALLET_FILTER_OTHER},
    {WALLET_LIST_HELIUM, &walletListHelium, true, WALLET_FILTER_SOL},
    {WALLET_LIST_BLUE, &walletListBlue, true, WALLET_FILTER_BTC},
    {WALLET_LIST_ZEUS, &walletListZeus, true, WALLET_FILTER_BTC},
    {WALLET_LIST_BABYLON, &walletListBabylon, true, WALLET_FILTER_BTC},
    {WALLET_LIST_SPARROW, &walletListSparrow, true, WALLET_FILTER_BTC},
    {WALLET_LIST_TONKEEPER, &walletListTonkeeper, false, WALLET_FILTER_OTHER},
    {WALLET_LIST_RABBY, &walletListRabby, true, WALLET_FILTER_ETH},
    {WALLET_LIST_BITGET, &walletListBitget, true, WALLET_FILTER_BTC | WALLET_FILTER_ETH | WALLET_FILTER_OTHER},
    {WALLET_LIST_ETERNL, &walletListEternl, true, WALLET_FILTER_ADA},
    {WALLET_LIST_VESPR, &walletListVespr, true, WALLET_FILTER_ADA},
    {WALLET_LIST_BEGIN, &walletListBegin, true, WALLET_FILTER_ADA},
    {WALLET_LIST_UNISAT, &walletListUniSat, true, WALLET_FILTER_BTC},
    {WALLET_LIST_SUIET, &walletListSuiet, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_IOTA, &walletListIota, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_NIGHTLY, &walletListNightly, true, WALLET_FILTER_OTHER},
    // {WALLET_LIST_YOROI, &walletListYoroi, true},
    {WALLET_LIST_TYPHON, &walletListTyphon, true, WALLET_FILTER_ADA},
    {WALLET_LIST_MEDUSA, &walletListMedusa, true, WALLET_FILTER_ADA},
    {WALLET_LIST_SAFE, &walletListSafe, true, WALLET_FILTER_ETH},
    {WALLET_LIST_BLOCK_WALLET, &walletListBlockWallet, true, WALLET_FILTER_ETH},
    {WALLET_LIST_XRP_TOOLKIT, &walletListXRPToolkit, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_THORWALLET, &walletListThorWallet, true, WALLET_FILTER_ETH | WALLET_FILTER_OTHER},
    {WALLET_LIST_PETRA, &walletListPetra, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_KEPLR, &walletListKeplr, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_LEAP, &walletListLeap, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_MINT_SCAN, &walletListMintScan, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_WANDER, &walletListWander, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_BEACON, &walletListBeacon, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_XBULL, &walletListXBull, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_IMTOKEN, &walletListImToken, true, WALLET_FILTER_ETH},
    {WALLET_LIST_FEWCHA, &walletListFewcha, true, WALLET_FILTER_OTHER},
    {WALLET_LIST_ZAPPER, &walletListZapper, true, WALLET_FILTER_ETH},
    {WALLET_LIST_YEARN_FINANCE, &walletListYearn, true, WALLET_FILTER_ETH},
    {WALLET_LIST_SUSHISWAP, &walletListSushi, true, WALLET_FILTER_ETH},
};

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

static const lv_img_dsc_t *g_bitgetWalletCoinArray[] = {
    &coinBtc, &coinEth, &coinTon
};

static const lv_img_dsc_t *g_backpackWalletCoinArray[3] = {
    &coinSol, &coinEth, &coinSui
};

static const lv_img_dsc_t *g_keystoneWalletCoinArray[] = {
    &coinBtc, &coinEth, &coinTrx, &coinXrp, &coinBnb, &coinDoge
};

static const lv_img_dsc_t *g_blueWalletCoinArray[4] = {
    &coinBtc,
};

static const lv_img_dsc_t *g_UniSatCoinArray[5] = {
    &coinBtc, &coinOrdi, &coinSats, &coinMubi, &coinTrac,
};

static const lv_img_dsc_t *g_keplrCoinArray[8] = {
    &coinAtom, &coinOsmo, &coinBld, &coinAkt,
    &coinXprt, &coinAxl, &coinBoot, &coinCro,
};

static const lv_img_dsc_t *g_leapCoinArray[8] = {
    &coinAtom, &coinOsmo, &coinInj, &coinStrd, &coinStars, &coinJuno, &coinScrt, &coinDym
};

static const lv_img_dsc_t *g_wanderCoinArray[1] = {
    &coinAr,
};

static const lv_img_dsc_t *g_beaconCoinArray[2] = {
    &coinAr,
    // todo add ao
    &coinAo,
};

static const lv_img_dsc_t *g_xbullCoinArray[1] = {
    &coinXlm,
};

static const lv_img_dsc_t *g_fewchaCoinArray[FEWCHA_COINS_BUTT] = {
    &coinApt,
    &coinSui,
};

static const lv_img_dsc_t *g_coreCoinArray[] = {
    &coinAva,
    &coinEth,
    &coinBtc,
};

static const lv_img_dsc_t *g_petraCoinArray[1] = {
    &coinApt,
};

static const lv_img_dsc_t *g_nightlyCoinArray[] = {
    &coinSui, &coinIota, &coinApt
};

static const lv_img_dsc_t *g_suiWalletCoinArray[] = {
    &coinSui
};

static const lv_img_dsc_t *g_iotaCoinArray[1] = {
    &coinIota,
};

static const lv_img_dsc_t *g_solfareCoinArray[1] = {
    &coinSol,
};

static const lv_img_dsc_t *g_heliumCoinArray[2] = {
    &coinSol,
    &coinHelium,
};

static const lv_img_dsc_t *g_tonKeeperCoinArray[1] = {
    &coinTon,
};

static const lv_img_dsc_t *g_ThorWalletCoinArray[3] = {
    // todo thorchain will support bitcoin later
    // &coinBtc,
    &coinEth,
    &coinRune,
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

const static ChangeDerivationItem_t g_adaChangeDerivationList[] = {
    {"Icarus", ""},
    {"Ledger/BitBox02", ""},
};

static const char *g_walletFilter[6] = {"All", "BTC", "ETH", "SOL", "ADA", "· · ·"};
static uint8_t g_currentFilter = WALLET_FILTER_ALL;

static uint32_t g_currentSelectedPathIndex[3] = {0};
static lv_obj_t *g_walletListCont = NULL;
static lv_obj_t *g_coinListCont = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;

static lv_obj_t *g_noticeWindow = NULL;
static ConnectWalletWidget_t g_connectWalletTileView;
static PageWidget_t *g_pageWidget;

static void UpdategAddress(void);
static void GetEgAddress(void);
static void GetEthEgAddress(void);
static void initFewchaCoinsConfig(void);
static void OpenQRCodeHandler(lv_event_t *e);
static void ReturnShowQRHandler(lv_event_t *e);
static void UpdateFewchaCoinStateHandler(lv_event_t *e);
static void JumpSelectCoinPageHandler(lv_event_t *e);
static void ConfirmSelectFewchaCoinsHandler(lv_event_t *e);
static void GuiCreateSelectFewchaCoinWidget();
void ConnectWalletReturnHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);
static void AddMetaMaskCoins(void);
static void AddOkxWalletCoins(void);
static void AddBlueWalletCoins(void);
static void AddFewchaCoins(void);
static void AddCoreWalletCoins(void);
static void AddSolflareCoins(void);
static void AddHeliumWalletCoins(void);
static void AddThorWalletCoins(void);
static void ShowEgAddressCont(lv_obj_t *egCont);
static uint32_t GetCurrentSelectedIndex();
static bool HasSelectAddressWidget();
static uint32_t GetDerivedPathTypeCount();
static int GetAccountType(void);

CoinState_t g_fewchaCoinState[FEWCHA_COINS_BUTT];
static char g_derivationPathAddr[LedgerLegacy + 1][DERIVATION_PATH_EG_LEN][64];
static char g_solDerivationPathAddr[SOLBip44Change + 1][DERIVATION_PATH_EG_LEN][64];
static char g_adaDerivationPathAddr[LEDGER_ADA + 1][DERIVATION_PATH_EG_LEN][64];
static lv_obj_t *g_derivationCheck[LedgerLegacy + 1];
static ETHAccountType g_currentEthPathIndex[3] = {Bip44Standard, Bip44Standard, Bip44Standard};
static SOLAccountType g_currentSOLPathIndex[3] = {SOLBip44, SOLBip44, SOLBip44};
static SOLAccountType g_currentHeliumPathIndex[3] = {SOLBip44, SOLBip44, SOLBip44};
static AdaXPubType g_currentAdaPathIndex[3] = {STANDARD_ADA, STANDARD_ADA, STANDARD_ADA};

static lv_obj_t *g_egAddress[DERIVATION_PATH_EG_LEN];
static lv_obj_t *g_egAddressIndex[DERIVATION_PATH_EG_LEN];

static CoinState_t g_tempFewchaCoinState[FEWCHA_COINS_BUTT];

static lv_obj_t *g_coinCont = NULL;
static lv_obj_t *g_coinTitleLabel = NULL;
static lv_obj_t *g_openMoreHintBox = NULL;
static lv_obj_t *g_bottomCont = NULL;
static lv_obj_t *g_manageImg = NULL;
static bool g_isCoinReselected = false;
static lv_obj_t *g_derivationPathCont = NULL;
static char **g_derivationPathDescs = NULL;
static lv_obj_t *g_derivationPathConfirmBtn = NULL;
static lv_obj_t *g_egCont = NULL;

static void QRCodePause(bool);
static void GuiInitWalletListArray()
{
    bool isTON = false;
    bool isSLIP39 = false;
    bool isTempAccount = false;
    bool isRussian = false;

    isTON = (GetMnemonicType() == MNEMONIC_TYPE_TON);
    isSLIP39 = (GetMnemonicType() == MNEMONIC_TYPE_SLIP39);
    isTempAccount = GetIsTempAccount();
    isRussian = (LanguageGetIndex() == LANG_RU);

    for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        bool enable = true;
        int index = 0;
        for (int j = 0; j < NUMBER_OF_ARRAYS(g_walletListArray); j++) {
            if (g_walletListArray[j].index == i) {
                index = j;
                break;
            }
        }

        if (isTON) {
            enable = (index == WALLET_LIST_TONKEEPER);
        } else {
            switch (index) {
            case WALLET_LIST_WANDER:
            case WALLET_LIST_BEACON:
                enable = !isTempAccount;
                break;
            // open keystone for test
            // case WALLET_LIST_KEYSTONE:
            //     enable = isRussian;
            //     break;
            default:
                break;
            }
        }
        g_walletListArray[i].enable = enable && (g_currentFilter & g_walletListArray[i].filter);
    }
}

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
    case WALLET_LIST_NUFI:
        return true;
    default:
        return false;
    }
}

static bool IsSOL(int walletIndex)
{
    switch (walletIndex) {
    case WALLET_LIST_SOLFARE:
    case WALLET_LIST_HELIUM:
    case WALLET_LIST_NUFI:
        return true;
    default:
        return false;
    }
}

static bool IsAda(int walletIndex)
{
    if (GetMnemonicType() == MNEMONIC_TYPE_SLIP39) {
        return false;
    }
    switch (walletIndex) {
    case WALLET_LIST_VESPR:
    case WALLET_LIST_ETERNL:
    case WALLET_LIST_MEDUSA:
    case WALLET_LIST_TYPHON:
    case WALLET_LIST_BEGIN:
        return true;
    default:
        return false;
    }
}

static void GuiARAddressCheckConfirmHandler(lv_event_t *event)
{
    GUI_DEL_OBJ(g_noticeWindow);
    GuiCreateAttentionHintbox(SIG_SETUP_RSA_PRIVATE_KEY_CONNECT_CONFIRM);
}

static void GuiOpenARAddressNoticeWindow()
{
    g_noticeWindow = GuiCreateGeneralHintBox(&imgWarn, _("ar_address_check"), _("ar_address_check_desc"), NULL, _("not_now"), WHITE_COLOR_OPA20, _("understand"), ORANGE_COLOR);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeWindow, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_set_width(btn, 192);
    lv_obj_add_event_cb(btn, GuiARAddressCheckConfirmHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    btn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_set_width(btn, 192);
    lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgClose);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_align_to(img, lv_obj_get_child(g_noticeWindow, 1), LV_ALIGN_TOP_RIGHT, -36, 36);
}


static void GuiOpenNufiNoticeWindow()
{
    g_noticeWindow = GuiCreateGeneralHintBox(&imgBlueInformation, _("nufi_connection_notice"), _("nufi_connection_notice_desc"), NULL, NULL, WHITE_COLOR_OPA20, _("understand"), ORANGE_COLOR);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeWindow, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    // understand button
    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_set_width(btn, 408);
    lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}


static void OpenQRCodeHandler(lv_event_t *e)
{
    WalletListItem_t *wallet = lv_event_get_user_data(e);
    g_connectWalletTileView.walletIndex = wallet->index;
    printf("OpenQRCodeHandler %d.........\r\n", g_connectWalletTileView.walletIndex);
    if (IsEVMChain(g_connectWalletTileView.walletIndex)) {
        g_derivationPathDescs = GetDerivationPathDescs(ETH_DERIVATION_PATH_DESC);
    }
    if (IsSOL(g_connectWalletTileView.walletIndex)) {
        g_derivationPathDescs = GetDerivationPathDescs(SOL_DERIVATION_PATH_DESC);
    }
    if (IsAda(g_connectWalletTileView.walletIndex)) {
        g_derivationPathDescs = GetDerivationPathDescs(ADA_DERIVATION_PATH_DESC);
    }
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_ETERNL ||
            g_connectWalletTileView.walletIndex == WALLET_LIST_TYPHON ||
            g_connectWalletTileView.walletIndex == WALLET_LIST_BEGIN ||
            g_connectWalletTileView.walletIndex == WALLET_LIST_MEDUSA
       ) {
        GuiCreateConnectADAWalletWidget(g_connectWalletTileView.walletIndex);
        return;
    }
    // todo: notice if current wallet is nufi , we show a hintbox to notify user to connect usb
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_NUFI) {
        GuiOpenNufiNoticeWindow();
        return;
    }
    bool skipGenerateArweaveKey = IsArweaveSetupComplete();
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_WANDER && !skipGenerateArweaveKey) {
        GuiOpenARAddressNoticeWindow();
        return;
    }
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_BEACON && !skipGenerateArweaveKey) {
        GuiOpenARAddressNoticeWindow();
        return;
    }
    g_isCoinReselected = false;
    printf("%s %d.........\r\n", __func__, __LINE__);
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
}

void GuiConnectWalletPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void GuiConnectShowRsaSetupasswordHintbox(void)
{
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

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
            SetConnectWalletNetwork(GetWalletNameByIndex(g_connectWalletTileView.walletIndex), i);
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
    if (GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex)) != index) {
        SetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex), index);
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
        uint8_t accountIndex = GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
        uint8_t chainType = 0;
        if (g_connectWalletTileView.walletIndex == WALLET_LIST_XRP_TOOLKIT || g_connectWalletTileView.walletIndex == WALLET_LIST_XAMAN) {
            chainType = CHAIN_XRP;
        } else if (g_connectWalletTileView.walletIndex == WALLET_LIST_VESPR) {
            chainType = CHAIN_ADA;
        }
        if (chainType != 0) {
            g_coinListCont = GuiCreateSelectAddressWidget(chainType, accountIndex, RefreshAddressIndex);
        }
    }
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_KEPLR || g_connectWalletTileView.walletIndex == WALLET_LIST_MINT_SCAN) {
        g_coinListCont = GuiCreateSelectAddressWidget(
                             CHAIN_ATOM, GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex)),
                             RefreshAddressIndex);
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

static void GuiUpdateWalletListWidget(void)
{
    lv_obj_clean(g_walletListCont);
    int offsetY = 0;
    for (int i = 0, j = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        if (!g_walletListArray[i].enable) {
            continue;
        }
        lv_obj_t *img = GuiCreateImg(g_walletListCont, g_walletListArray[i].img);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, offsetY);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, OpenQRCodeHandler, LV_EVENT_CLICKED,
                            &g_walletListArray[i]);
        j++;
        offsetY = j * 107;
    }
}

static void GuiUpdateConnectWalletHandler(lv_event_t *e)
{
    char *selectFilter = lv_event_get_user_data(e);
    int i = 0;
    lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));
    for (i = 0; i < NUMBER_OF_ARRAYS(g_walletFilter); i++) {
        if (strcmp(g_walletFilter[i], selectFilter) == 0) {
            uint8_t currentFilter = (i == 0) ? WALLET_FILTER_ALL : (1 << (i - 1));
            if (currentFilter == g_currentFilter) {
                return;
            }
            g_currentFilter = currentFilter;
            lv_obj_set_style_border_width(lv_obj_get_child(parent, i), 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        }
        lv_obj_set_style_border_width(lv_obj_get_child(parent, i), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    for (++i; i < NUMBER_OF_ARRAYS(g_walletFilter); i++) {
        lv_obj_set_style_border_width(lv_obj_get_child(parent, i), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_set_style_border_color(obj, RED_COLOR, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    GuiInitWalletListArray();
    GuiUpdateWalletListWidget();
}

static void GuiCreateSelectWalletWidget(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    bool isTon = GetMnemonicType() == MNEMONIC_TYPE_TON;
    bool isSlip39 = GetMnemonicType() == MNEMONIC_TYPE_SLIP39;
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
        lv_obj_t *filterBar = GuiCreateContainerWithParent(parent, 408, 64);
        lv_obj_align(filterBar, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_flex_flow(filterBar, LV_FLEX_FLOW_ROW);

        for (int i = 0; i < NUMBER_OF_ARRAYS(g_walletFilter); i++) {
            lv_obj_t *btn = GuiCreateBtnWithFont(filterBar, g_walletFilter[i], &openSansEnIllustrate);
            lv_obj_set_size(btn, 40, 64);
            lv_obj_set_style_radius(btn, 0, 0);
            lv_obj_set_style_bg_color(btn, BLACK_COLOR, 0);
            lv_obj_set_flex_grow(btn, 1);
            lv_obj_set_style_border_color(btn, ORANGE_COLOR, 0);
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(btn, i == 0 ? 2 : 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(btn, GuiUpdateConnectWalletHandler, LV_EVENT_CLICKED, g_walletFilter[i]);
        }

        lv_obj_t *line = GuiCreateDividerLine(parent);
        lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 64);

        lv_obj_t *walletListCont = GuiCreateContainerWithParent(parent, 480, 656 - 64 - 1);
        g_walletListCont = walletListCont;
        lv_obj_align(walletListCont, LV_ALIGN_TOP_MID, 0, 65);
        lv_obj_add_flag(walletListCont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(walletListCont, LV_OBJ_FLAG_SCROLL_ELASTIC);
        lv_obj_set_scrollbar_mode(walletListCont, LV_SCROLLBAR_MODE_OFF);
        GuiUpdateWalletListWidget();
    }
}

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

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_scan"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 490);
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

    g_bottomCont = GuiCreateContainerWithParent(qrCont, 408, 124);
    lv_obj_align(g_bottomCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(g_bottomCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_bottomCont, LV_OPA_0,
                            LV_STATE_DEFAULT | LV_PART_MAIN);
    // GuiCreateSupportedNetworks(g_connectWalletTileView.walletIndex);
}

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

static void AddBitgetWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0;
            i < sizeof(g_bitgetWalletCoinArray) / sizeof(g_bitgetWalletCoinArray[0]);
            i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_bitgetWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddKeystoneWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0;
            i < sizeof(g_keystoneWalletCoinArray) / sizeof(g_keystoneWalletCoinArray[0]);
            i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_keystoneWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
    // lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    // lv_img_set_zoom(img, 150);
    // lv_img_set_pivot(img, 0, 0);
    // lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    // lv_obj_align(img, LV_ALIGN_TOP_LEFT, 132, 2);
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

static void AddWanderConnectCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, g_wanderCoinArray[0]);
    lv_img_set_zoom(img, 110);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);
}

static void AddBeaconCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }

    for (int i = 0; i < 2; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_beaconCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddXBullCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, g_xbullCoinArray[0]);
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

static void AddCoreWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < CORE_COINS_BUTT; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_coreCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
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

static void AddThorWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 3; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_ThorWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddWalletCoins(const char *coinArray[], uint32_t arraySize)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (uint32_t i = 0; i < arraySize; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, coinArray[i]);
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
    int index = GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
    printf("%s %d..\n", __func__, __LINE__);
    snprintf_s(name, sizeof(name), "%s-%d", _("account_head"), index);
    lv_obj_t *label = GuiCreateIllustrateLabel(g_bottomCont, name);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 24);

    if (g_connectWalletTileView.walletIndex == WALLET_LIST_XRP_TOOLKIT || g_connectWalletTileView.walletIndex == WALLET_LIST_XAMAN) {
        char addr[BUFFER_SIZE_256] = {0};
        CutAndFormatString(addr, sizeof(addr), GuiGetXrpAddressByIndex(index), 20);
        label = GuiCreateNoticeLabel(g_bottomCont, addr);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 58);
    } else if (IsAda(g_connectWalletTileView.walletIndex)) {
        char addr[BUFFER_SIZE_256] = {0};
        char *xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndexAndDerivationType(
                GetConnectWalletPathIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex)), index));
        CutAndFormatString(addr, sizeof(addr), GuiGetADABaseAddressByXPub(xpub), 20);
        label = GuiCreateNoticeLabel(g_bottomCont, addr);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 58);
    }

    label = GuiCreateImg(g_bottomCont, &imgArrowRight);
    lv_obj_align(label, LV_ALIGN_CENTER, 150, 0);
}

static void AddKeplrCoinsAndAddressUI(void)
{
    if (lv_obj_get_child_cnt(g_bottomCont) > 0 && g_bottomCont != NULL) {
        lv_obj_clean(g_bottomCont);
        g_manageImg = NULL;
        g_coinCont = NULL;
    }
    lv_obj_add_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    g_coinTitleLabel = GuiCreateNoticeLabel(g_bottomCont, _("connect_wallet_supported_networks"));
    lv_label_set_text(g_coinTitleLabel, _("connect_wallet_supported_networks"));
    lv_obj_align(g_coinTitleLabel, LV_ALIGN_TOP_LEFT, 36, 10);
    // supported network icons
    for (int i = 0; i < 8; i++) {
        lv_obj_t *img = GuiCreateImg(g_bottomCont, g_keplrCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 36 + 32 * i, 38);
    }

    lv_obj_t *img = GuiCreateImg(g_bottomCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 292, 38);

    // select address ui
    lv_obj_add_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    char name[BUFFER_SIZE_32] = {0};
    snprintf_s(name, sizeof(name), "%s-%d", _("account_head"),
               GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex)));
    lv_obj_t *label = GuiCreateIllustrateLabel(g_bottomCont, name);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 70);

    label = GuiCreateImg(g_bottomCont, &imgArrowRight);
    lv_obj_align(label, LV_ALIGN_CENTER, 150, 30);
}

static void AddLeapCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 8; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_leapCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
    // Add more
    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * 8, 2);
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

static void AddHeliumWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }

    for (int i = 0; i < 2; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_heliumCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddBackpackWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0;
            i < sizeof(g_backpackWalletCoinArray) / sizeof(g_backpackWalletCoinArray[0]);
            i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_backpackWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

void GuiConnectWalletInit(void)
{
    g_currentFilter = WALLET_FILTER_ALL;
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

UREncodeResult *GuiGetFewchaData(void)
{
    GuiChainCoinType coin = CHAIN_APT;
    if (g_fewchaCoinState[1].state) {
        coin = CHAIN_SUI;
    }
    return GuiGetFewchaDataByCoin(coin);
}

UREncodeResult *GuiGetNightlyData(void)
{
    return GuiGetWalletDataByCoin(true);
}

UREncodeResult *GuiGetSuiWalletData(void)
{
    return GuiGetWalletDataByCoin(false);
}

UREncodeResult *GuiGetXrpToolkitData(void)
{
    printf("%s %d..\n", __func__, __LINE__);
    return GuiGetXrpToolkitDataByIndex(
               GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex)));
}
UREncodeResult *GuiGetKeplrData(void)
{
    return GuiGetKeplrDataByIndex(GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex)));
}

UREncodeResult *GuiGetADAData(void)
{
    return GuiGetADADataByIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
}

UREncodeResult *GuiGetTonData(void)
{
    bool isTon = GetMnemonicType() == MNEMONIC_TYPE_TON;
    uint8_t* mfp = NULL;
    char* path = NULL;
    char* xpub;
    if (isTon) {
        xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TON_NATIVE);
    } else {
        mfp = SRAM_MALLOC(4);
        GetMasterFingerPrint(mfp);
        xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TON_BIP39);
        path = GetXPubPath(XPUB_TYPE_TON_BIP39);
    }
    char* walletName = GetWalletName();
    if (walletName == NULL) {
        walletName = "Keystone";
    }
    return get_tonkeeper_wallet_ur(xpub, walletName, mfp, mfp == NULL ? 0 : 4, path);
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

void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index)
{
    GenerateUR func = NULL;
    SetWallet(g_pageWidget->navBarWidget, index, NULL);
    GuiCreateSupportedNetworks(index);
    lv_label_set_text(g_coinTitleLabel, _("connect_wallet_supported_networks"));
    lv_obj_clear_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(g_manageImg, LV_OBJ_FLAG_HIDDEN);
    switch (index) {
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
    case WALLET_LIST_CORE:
        func = GuiGetCoreWalletData;
        AddCoreWalletCoins();
        break;
    case WALLET_LIST_BITGET:
        func = GuiGetBitgetWalletData;
        AddBitgetWalletCoins();
        break;
    case WALLET_LIST_OKX:
        func = GuiGetOkxWalletData;
        AddOkxWalletCoins();
        break;

    case WALLET_LIST_BLUE:
        func = GuiGetBlueWalletBtcData;
        AddBlueWalletCoins();
        break;
    // todo  zeus wallet use same ur logic as sparrow wallet (m/49'/0'/0' 、 m/44'/0'/0' 、 m/84'/0'/0' and m/86'/0'/0' )
    case WALLET_LIST_ZEUS:
    case WALLET_LIST_SPARROW:
    case WALLET_LIST_BABYLON:
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
    case WALLET_LIST_HELIUM:
        func = GuiGetHeliumData;
        AddHeliumWalletCoins();
        break;
    case WALLET_LIST_BACKPACK:
        func = GuiGetBackpackData;
        AddBackpackWalletCoins();
        break;
    case WALLET_LIST_MINT_SCAN:
    case WALLET_LIST_KEPLR:
        func = GuiGetKeplrData;
        AddKeplrCoinsAndAddressUI();
        break;
    case WALLET_LIST_LEAP:
        func = GuiGetLeapData;
        AddLeapCoins();
        break;
    case WALLET_LIST_WANDER:
        func = GuiGetWanderData;
        AddWanderConnectCoins();
        break;
    case WALLET_LIST_BEACON:
        func = GuiGetWanderData;
        AddBeaconCoins();
        break;
    case WALLET_LIST_XBULL:
        func = GuiGetXBullData;
        AddXBullCoins();
        break;
    case WALLET_LIST_TYPHON:
        func = GuiGetADAData;
        AddChainAddress();
        break;
    case WALLET_LIST_VESPR:
        func = GuiGetADAData;
        AddChainAddress();
        break;
    case WALLET_LIST_NIGHTLY:
        func = GuiGetNightlyData;
        AddWalletCoins(g_nightlyCoinArray, NUMBER_OF_ARRAYS(g_nightlyCoinArray));
        break;
    case WALLET_LIST_IOTA:
        func = GuiGetIotaWalletData;
        AddWalletCoins(g_iotaCoinArray, NUMBER_OF_ARRAYS(g_iotaCoinArray));
        break;
    case WALLET_LIST_SUIET:
        func = GuiGetSuiWalletData;
        AddWalletCoins(g_suiWalletCoinArray, NUMBER_OF_ARRAYS(g_suiWalletCoinArray));
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
    case WALLET_LIST_XAMAN:
        printf("%s %d..\n", __func__, __LINE__);
        func = GuiGetXrpToolkitData;
        AddChainAddress();
        break;
    case WALLET_LIST_XRP_TOOLKIT:
        func = GuiGetXrpToolkitData;
        AddChainAddress();
        break;
    case WALLET_LIST_THORWALLET:
        func = GuiGetThorWalletBtcData;
        AddThorWalletCoins();
        break;
    case WALLET_LIST_TONKEEPER:
        func = GuiGetTonData;
        AddTonCoins();
        break;
    case WALLET_LIST_KEYSTONE:
        // todo  add keystone ur logic
        func = GuiGetKeystoneConnectWalletData;
        AddKeystoneWalletCoins();
        break;
    default:
        return;
    }
    if (func) {
        bool skipGenerateArweaveKey = IsArweaveSetupComplete();
        if (index == WALLET_LIST_WANDER && !skipGenerateArweaveKey) {
            GuiAnimatingQRCodeInitWithLoadingParams(g_connectWalletTileView.qrCode, func, true, _("InitializingRsaTitle"), _("FindingRsaPrimes"));
            return;
        }
        if (index == WALLET_LIST_BEACON && !skipGenerateArweaveKey) {
            GuiAnimatingQRCodeInitWithLoadingParams(g_connectWalletTileView.qrCode, func, true, _("InitializingRsaTitle"), _("FindingRsaPrimes"));
            return;
        }
        GuiAnimatingQRCodeInit(g_connectWalletTileView.qrCode, func, true);
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

static void initFewchaCoinsConfig(void)
{
    uint8_t index = GetConnectWalletNetwork(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_defaultFewchaState); i++) {
        if (i == index) {
            g_defaultFewchaState[index].state = true;
        } else {
            g_defaultFewchaState[i].state = false;
        }
    }
    memcpy(g_tempFewchaCoinState, g_defaultFewchaState,
           sizeof(g_defaultFewchaState));
    memcpy(g_fewchaCoinState, g_defaultFewchaState, sizeof(g_defaultFewchaState));
}

ETHAccountType GetMetamaskAccountType(void)
{
    return GetConnectWalletPathIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
}

SOLAccountType GetSolflareAccountType(void)
{
    return GetConnectWalletPathIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
}

SOLAccountType GetHeliumAccountType(void)
{
    return GetConnectWalletPathIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
}

static int GetAccountType(void)
{
    return GetConnectWalletPathIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
}

static void SetAccountType(uint8_t index)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        g_currentSOLPathIndex[GetCurrentAccountIndex()] = index;
        break;
    case WALLET_LIST_HELIUM:
        g_currentHeliumPathIndex[GetCurrentAccountIndex()] = index;
        break;
    case WALLET_LIST_VESPR:
        g_currentAdaPathIndex[GetCurrentAccountIndex()] = index;
        break;
    default:
        g_currentEthPathIndex[GetCurrentAccountIndex()] = index;
        break;
    }

    SetConnectWalletPathIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex), index);
}

static bool IsSelectChanged(void)
{
    return GetCurrentSelectedIndex() != GetAccountType();
}

static bool HasSelectAddressWidget()
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_XRP_TOOLKIT:
    case WALLET_LIST_XAMAN:
    case WALLET_LIST_KEPLR:
    case WALLET_LIST_VESPR:
    case WALLET_LIST_MINT_SCAN:
        return true;
        break;
    default:
        return false;
    }
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

static void GetAdaEgAddress(void)
{
    ChainType chainType1 = GetCurrentSelectedIndex() == STANDARD_ADA ? XPUB_TYPE_ADA_0 : XPUB_TYPE_LEDGER_ADA_0;
    ChainType chainType2 = GetCurrentSelectedIndex() == STANDARD_ADA ? XPUB_TYPE_ADA_1 : XPUB_TYPE_LEDGER_ADA_1;
    SimpleResponse_c_char *result;
    result =
        cardano_get_base_address(GetCurrentAccountPublicKey(chainType1), 0, 1);
    CutAndFormatString(g_adaDerivationPathAddr[GetCurrentSelectedIndex()][0], BUFFER_SIZE_128,
                       result->data, 24);
    free_simple_response_c_char(result);

    result =
        cardano_get_base_address(GetCurrentAccountPublicKey(chainType2), 0, 1);
    CutAndFormatString(g_adaDerivationPathAddr[GetCurrentSelectedIndex()][1], BUFFER_SIZE_128,
                       result->data, 24);
    free_simple_response_c_char(result);
}

static void GetEgAddress(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
        GetSolEgAddress();
        break;
    case WALLET_LIST_HELIUM:
        GetSolEgAddress();
        break;
    case WALLET_LIST_VESPR:
        GetAdaEgAddress();
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

static void UpdateAdaEgAddress(uint8_t index)
{
    lv_label_set_text(g_egAddress[0],
                      (const char *)g_adaDerivationPathAddr[index][0]);
    lv_label_set_text(g_egAddress[1],
                      (const char *)g_adaDerivationPathAddr[index][1]);
}

static void UpdategAddress(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
    case WALLET_LIST_HELIUM:
        UpdateSolEgAddress(GetCurrentSelectedIndex());
        break;
    case WALLET_LIST_VESPR:
        UpdateAdaEgAddress(GetCurrentSelectedIndex());
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

static uint32_t GetCurrentSelectedIndex()
{
    return g_currentSelectedPathIndex[GetCurrentAccountIndex()];
}

static void SelectDerivationHandler(lv_event_t *e)
{
    lv_obj_t *newCheckBox = lv_event_get_user_data(e);
    for (int i = 0; i < GetDerivedPathTypeCount(); i++) {
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

static void OpenTutorialHandler(lv_event_t *e)
{
    QRCodePause(true);

    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(wallet));
    GUI_DEL_OBJ(g_openMoreHintBox);
}

static const char *GetDerivationPathSelectDes(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_SOLFARE:
    case WALLET_LIST_HELIUM:
        return _("derivation_path_select_sol");
    case WALLET_LIST_VESPR:
        return _("derivation_path_select_ada");
    default:
        return _("derivation_path_select_eth");
    }
}

static const char *GetChangeDerivationAccountType(int i)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_VESPR:
        if (i == 0) {
            return _("receive_ada_more_t_standard");
        } else if (i == 1) {
            return _("receive_ada_more_t_ledger");
        }
    case WALLET_LIST_SOLFARE:
    case WALLET_LIST_HELIUM:
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
    case WALLET_LIST_HELIUM:
        return g_solChangeDerivationList[i].path;
    case WALLET_LIST_VESPR:
        return g_adaChangeDerivationList[i].path;
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
    // if current wallet is  solana wallet and the current selected index is not root path, then show the second example address
    if ((g_connectWalletTileView.walletIndex == WALLET_LIST_SOLFARE || g_connectWalletTileView.walletIndex == WALLET_LIST_HELIUM) &&
            GetCurrentSelectedIndex() != SOLBip44ROOT) {
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
    // not solana wallet, show the second example address
    if (g_connectWalletTileView.walletIndex != WALLET_LIST_SOLFARE && g_connectWalletTileView.walletIndex != WALLET_LIST_HELIUM) {
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

static uint32_t GetDerivedPathTypeCount()
{
    if (IsAda(g_connectWalletTileView.walletIndex)) {
        return 2;
    } else {
        return 3;
    }
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

    lv_obj_t *label = GuiCreateNoticeLabel(scrollCont, GetDerivationPathSelectDes());
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
    uint16_t buttonHeight = IsAda(g_connectWalletTileView.walletIndex) ? 68 : 102;
    lv_obj_t *cont = GuiCreateContainerWithParent(scrollCont, 408, (buttonHeight + 1) * GetDerivedPathTypeCount() - 1);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    for (int i = 0; i < GetDerivedPathTypeCount(); i++) {
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
            GuiCreateButton(cont, 408, buttonHeight, table, NUMBER_OF_ARRAYS(table),
                            SelectDerivationHandler, g_derivationCheck[i]);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, i * buttonHeight);
        if (i != 0) {
            static lv_point_t points[2] = {{0, 0}, {360, 0}};
            lv_obj_t *line = (lv_obj_t *)GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, i * buttonHeight);
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

static void OpenMoreHandler(lv_event_t *e)
{
    int hintboxHeight = 132;
    lv_obj_t *btn = NULL;
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    bool isSpeciaWallet = IsEVMChain(*wallet) || IsSOL(*wallet) || IsAda(*wallet);
    if (isSpeciaWallet) {
        hintboxHeight = 228;
    }
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0),
                        CloseHintBoxHandler, LV_EVENT_CLICKED,
                        &g_openMoreHintBox);
    btn = GuiCreateSelectButton(g_openMoreHintBox, _("Tutorial"), &imgTutorial,
                                OpenTutorialHandler, wallet, true);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    if (isSpeciaWallet) {
        hintboxHeight = 228;
        btn = GuiCreateSelectButton(g_openMoreHintBox, _("derivation_path_change"), &imgPath, ChangeDerivationPathHandler, wallet, true);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -120);
    }
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
        if (g_coinListCont != NULL) {
            if (g_connectWalletTileView.walletIndex == WALLET_LIST_FEWCHA) {
                GUI_DEL_OBJ(g_coinListCont)
                GuiCreateSelectFewchaCoinWidget();
            } else if (HasSelectAddressWidget()) {
                GuiDestroySelectAddressWidget();
                uint8_t accountIndex = GetConnectWalletAccountIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
                uint8_t chainType;
                bool shouldCreate = false;

                switch (g_connectWalletTileView.walletIndex) {
                case WALLET_LIST_XRP_TOOLKIT:
                case WALLET_LIST_XAMAN:
                    chainType = CHAIN_XRP;
                    shouldCreate = true;
                    break;
                case WALLET_LIST_KEPLR:
                case WALLET_LIST_MINT_SCAN:
                    chainType = CHAIN_ATOM;
                    shouldCreate = true;
                    break;
                default:
                    shouldCreate = false;
                    break;
                }

                if (shouldCreate) {
                    g_coinListCont = GuiCreateSelectAddressWidget(chainType, accountIndex, RefreshAddressIndex);
                }
            }
        }
        QRCodePause(false);
    }
    if (ConnectADAWidgetExist()) {
        CleanConnectADAWidget();
        GuiCreateConnectADAWalletWidget(g_connectWalletTileView.walletIndex);
    }
    if (g_derivationPathCont != NULL) {
        GUI_DEL_OBJ(g_derivationPathCont);
        OpenDerivationPath();
    }
}
void GuiConnectWalletDeInit(void)
{
    GUI_DEL_OBJ(g_openMoreHintBox)
    GUI_DEL_OBJ(g_manageImg);
    GUI_DEL_OBJ(g_coinCont)
    GUI_DEL_OBJ(g_derivationPathCont)
    GUI_DEL_OBJ(g_noticeWindow)
    if (g_coinListCont != NULL && HasSelectAddressWidget()) {
        g_coinListCont = NULL;
        GuiDestroySelectAddressWidget();
    } else {
        GUI_DEL_OBJ(g_coinListCont)
    }
    CleanConnectADAWidget();
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