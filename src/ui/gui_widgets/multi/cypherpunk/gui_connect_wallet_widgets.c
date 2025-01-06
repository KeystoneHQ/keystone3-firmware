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
#include "gui_keyboard_hintbox.h"
#include "gui_pending_hintbox.h"
#include "account_manager.h"
#include "gui_animating_qrcode.h"
#include "gui_global_resources.h"
#include "gui_page.h"
#include "keystore.h"
#include "gui_select_address_widgets.h"
#include "account_public_info.h"

static bool IsSupportEncryption(void);
static void PrivateModeQRSharingHandler(lv_event_t *e);
static void ShowOrHiddenPincode(lv_event_t *e);
static void RestorePrivateMode(void);
static void ExitPrivateMode(void);
static void PrivateModePrevTileHandler(lv_event_t *e);
static void GuiCreateQrCodePrivateModeWidget(lv_obj_t *parent);
static void CancelAttentionHandler(lv_event_t *e);
static void CloseAttentionHandler(lv_event_t *e);
static void AddCakeCoins(void);

#define DERIVATION_PATH_EG_LEN 2
#define HIDDEN_PINCODE "* * * * * *"

typedef enum {
    CONNECT_WALLET_SELECT_WALLET = 0,
    CONNECT_WALLET_QRCODE,
    CONNECT_WALLET_QRCODE_PRIVATE_MODE,

    CONNECT_WALLET_BUTT,
} CONNECT_WALLET_ENUM;

typedef struct ConnectWalletWidget {
    uint32_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    WALLET_LIST_INDEX_ENUM walletIndex;
    lv_obj_t *qrCode;
    lv_obj_t *privateModeQrCode;
    lv_obj_t *privateModeQrCodeMask;
    lv_obj_t *privateModePincode;
    lv_obj_t *privateModePincodeBtn;
} ConnectWalletWidget_t;


WalletListItem_t g_walletListArray[] = {
    {WALLET_LIST_BLUE, &walletListBlue, true},
    {WALLET_LIST_SPARROW, &walletListSparrow, true},
    {WALLET_LIST_UNISAT, &walletListUniSat, true},
    {WALLET_LIST_ZEUS, &walletListZeus, true},
    {WALLET_LIST_CAKE, &walletListCake, true},
    {WALLET_LIST_FEATHER, &walletListFeather, true},
    {WALLET_LIST_ZASHI, &walletListZashi, true},
};
typedef struct {
    int8_t index;
    const char *coin;
    const lv_img_dsc_t *icon;
} CoinCard_t;

static const lv_img_dsc_t *g_blueWalletCoinArray[4] = {
    &coinBtc,
};

static const lv_img_dsc_t *g_UniSatCoinArray[5] = {
    &coinBtc, &coinOrdi, &coinSats, &coinMubi, &coinTrac,
};

static const lv_img_dsc_t *g_cakeCoinArray[1] = {
    &coinXmr,
};

static const lv_img_dsc_t *g_zashiCoinArray[1] = {
    &coinZec,
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

static uint32_t g_currentSelectedPathIndex[3] = {0};
static lv_obj_t *g_coinListCont = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;
static bool g_isNeedCloseTimer = false;
static lv_obj_t *g_privateModeHintBox = NULL;
static bool g_isFirstOpenPrivateMode = true;
static uint8_t *g_privateModePincode = NULL;
static lv_obj_t *g_noticeWindow = NULL;
static ConnectWalletWidget_t g_connectWalletTileView;
static PageWidget_t *g_pageWidget;

static void UpdategAddress(void);
static void GetEgAddress(void);
static void OpenQRCodeHandler(lv_event_t *e);
static void JumpSelectCoinPageHandler(lv_event_t *e);
void ConnectWalletReturnHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);
static uint32_t GetCurrentSelectedIndex();
static bool HasSelectAddressWidget();
static void ShowEgAddressCont(lv_obj_t *egCont);
static uint32_t GetDerivedPathTypeCount();
static int GetAccountType(void);

static lv_obj_t *g_derivationCheck[3];
static lv_obj_t *g_egAddress[DERIVATION_PATH_EG_LEN];
static lv_obj_t *g_egAddressIndex[DERIVATION_PATH_EG_LEN];

static lv_obj_t *g_coinCont = NULL;
static lv_obj_t *g_coinTitleLabel = NULL;
static lv_obj_t *g_openMoreHintBox = NULL;
static lv_obj_t *g_bottomCont = NULL;
static bool g_isCoinReselected = false;
static lv_obj_t *g_derivationPathCont = NULL;
static char **g_derivationPathDescs = NULL;
static lv_obj_t *g_derivationPathConfirmBtn = NULL;
static lv_obj_t *g_egCont = NULL;

static void QRCodePause(bool);

static void GuiInitWalletListArray()
{
    bool isSLIP39 = false;

    isSLIP39 = (GetMnemonicType() == MNEMONIC_TYPE_SLIP39);

    for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        bool enable = true;
        int index = g_walletListArray[i].index;

        bool passphraseExist = PassphraseExist(GetCurrentAccountIndex());
        MnemonicType mnemonicType = GetMnemonicType();
        bool isSlip39 = (mnemonicType == MNEMONIC_TYPE_SLIP39);

        switch (index) {
        case WALLET_LIST_CAKE:
        case WALLET_LIST_FEATHER:
            enable = !isSLIP39;
            break;
        case WALLET_LIST_ZASHI:
            enable = !passphraseExist && !isSlip39;
            break;
        default:
            break;
        }
        g_walletListArray[i].enable = enable;
    }
}

static void OpenQRCodeHandler(lv_event_t *e)
{
    WalletListItem_t *wallet = lv_event_get_user_data(e);
    g_connectWalletTileView.walletIndex = wallet->index;
    g_isCoinReselected = false;
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

static void JumpSelectCoinPageHandler(lv_event_t *e)
{
    if (g_coinListCont != NULL) {
        return;
    }
#ifndef COMPILE_SIMULATOR
    QRCodePause(true);
#endif
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_FEWCHA) {
    } else if (HasSelectAddressWidget()) {
    }
    if (g_connectWalletTileView.walletIndex == WALLET_LIST_KEPLR || g_connectWalletTileView.walletIndex == WALLET_LIST_MINT_SCAN) {
    }
}

static void GuiCreateSelectWalletWidget(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    int offsetY = 0;
    for (int i = 0, j = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        if (!g_walletListArray[i].enable) {
            continue;
        }
        lv_obj_t *img = GuiCreateImg(parent, g_walletListArray[i].img);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, offsetY);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, OpenQRCodeHandler, LV_EVENT_CLICKED,
                            &g_walletListArray[i]);
        j++;
        offsetY = j  * 107;
    }
}

static void GuiCreateSupportedNetworks(uint8_t index)
{
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

static void AddZecCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 1; i++) {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_zashiCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddCakeCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0) {
        lv_obj_clean(g_coinCont);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, g_cakeCoinArray[0]);
    lv_img_set_zoom(img, 110);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);
}

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

    tile = lv_tileview_add_tile(tileView, CONNECT_WALLET_QRCODE_PRIVATE_MODE, 0, LV_DIR_HOR);
    GuiCreateQrCodePrivateModeWidget(tile);

    g_connectWalletTileView.currentTile = CONNECT_WALLET_SELECT_WALLET;
    g_connectWalletTileView.tileView = tileView;
    g_connectWalletTileView.cont = cont;

    lv_obj_set_tile_id(g_connectWalletTileView.tileView,
                       g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
}

UREncodeResult *GuiGetZecData(void)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    CSliceFFI_ZcashKey *keys = SRAM_MALLOC(sizeof(CSliceFFI_ZcashKey));
    ZcashKey data[1];
    keys->data = data;
    keys->size = 1;
    char ufvk[384] = {'\0'};
    uint8_t sfp[32];
    GetZcashUFVK(GetCurrentAccountIndex(), ufvk, sfp);
    data[0].key_text = ufvk;
    data[0].key_name = GetWalletName();
    data[0].index = 0;
    return get_connect_zcash_wallet_ur(sfp, 32, keys);
}

void GuiPrepareArConnectWalletView(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
}

void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index)
{
    GuiAnimatingQRCodeDestroyTimer();
    GenerateUR func = NULL;
    SetWallet(g_pageWidget->navBarWidget, index, NULL);
    GuiCreateSupportedNetworks(index);
    lv_label_set_text(g_coinTitleLabel, _("connect_wallet_supported_networks"));
    lv_obj_clear_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    switch (index) {
    case WALLET_LIST_BLUE:
        func = GuiGetBlueWalletBtcData;
        AddBlueWalletCoins();
        break;
    // todo  zeus wallet use same ur logic as sparrow wallet (m/49'/0'/0' 、 m/44'/0'/0' 、 m/84'/0'/0' and m/86'/0'/0' )
    case WALLET_LIST_ZEUS:
    case WALLET_LIST_SPARROW:
        func = GuiGetSparrowWalletBtcData;
        AddBlueWalletCoins();
        break;
    case WALLET_LIST_UNISAT:
        func = GuiGetSparrowWalletBtcData;
        AddUniSatWalletCoins();
        lv_label_set_text(g_coinTitleLabel, _("connect_wallet_supported_tokens"));
        break;
    case WALLET_LIST_ZASHI:
        func = GuiGetZecData;
        AddZecCoins();
        break;
    case WALLET_LIST_CAKE:
    case WALLET_LIST_FEATHER:
        func = GuiGetCakeData;
        AddCakeCoins();
        break;
    default:
        return;
    }
    if (func) {
        lv_obj_t *qrcode = g_connectWalletTileView.qrCode;
        if (IsPrivateQrMode()) {
            qrcode = g_connectWalletTileView.privateModeQrCode;
        }
        GuiAnimatingQRCodeInit(qrcode, func, true);
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

static int GetAccountType(void)
{
    return GetConnectWalletPathIndex(GetWalletNameByIndex(g_connectWalletTileView.walletIndex));
}

static void SetAccountType(uint8_t index)
{
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

static void GetEgAddress(void)
{
}

static void UpdateEthEgAddress(uint8_t index)
{
}

static void UpdategAddress(void)
{
    UpdateEthEgAddress(GetCurrentSelectedIndex());
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
    egContHeight += 12;
    lv_obj_set_height(egCont, egContHeight);
    GetEgAddress();
    UpdategAddress();
}

static uint32_t GetDerivedPathTypeCount()
{
    return 3;
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
    uint16_t buttonHeight = 102;
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

static void OpenMoreHandler(lv_event_t *e)
{
    int hintboxHeight = 132;
    lv_obj_t *btn = NULL;
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);

    if (IsSupportEncryption()) {
        hintboxHeight = 228;
    }

    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0),
                        CloseHintBoxHandler, LV_EVENT_CLICKED,
                        &g_openMoreHintBox);
    btn = GuiCreateSelectButton(g_openMoreHintBox, _("Tutorial"), &imgTutorial,
                                OpenTutorialHandler, wallet, true);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);

    if (IsSupportEncryption()) {
        hintboxHeight = 228;
        btn = GuiCreateSelectButton(g_openMoreHintBox, _("private_mode_qr"), &imgQrcode36px, PrivateModeQRSharingHandler, wallet, true);
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
        if (IsSupportEncryption()) {
            SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                             PrivateModePrevTileHandler, NULL);
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                              OpenMoreHandler, &g_connectWalletTileView.walletIndex);
            g_privateModePincode = OpenPrivateQrMode();
            GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
        } else {
            return 0;
        }
        break;
    case CONNECT_WALLET_QRCODE_PRIVATE_MODE:
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
    case CONNECT_WALLET_QRCODE_PRIVATE_MODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler,
                         NULL);
        if (IsPrivateQrMode()) {
            ExitPrivateMode();
        }
        if (g_isNeedCloseTimer) {
            GuiAnimatingQRCodeDestroyTimer();
        } else {
            GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
        }
        g_isNeedCloseTimer = false;
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
    case CONNECT_WALLET_QRCODE_PRIVATE_MODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                         PrivateModePrevTileHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                          OpenMoreHandler, &g_connectWalletTileView.walletIndex);
        SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex, NULL);
        break;
    case CONNECT_WALLET_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                         ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                          OpenMoreHandler, &g_connectWalletTileView.walletIndex);
        SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex,
                  NULL);
        QRCodePause(false);
    }
    if (g_derivationPathCont != NULL) {
        GUI_DEL_OBJ(g_derivationPathCont);
        OpenDerivationPath();
    }
}
void GuiConnectWalletDeInit(void)
{
    GUI_DEL_OBJ(g_openMoreHintBox)
    GUI_DEL_OBJ(g_coinCont)
    GUI_DEL_OBJ(g_derivationPathCont)
    GUI_DEL_OBJ(g_noticeWindow)
    CloseAttentionHandler(NULL);
    if (g_coinListCont != NULL && HasSelectAddressWidget()) {
        g_coinListCont = NULL;
        GuiDestroySelectAddressWidget();
    } else {
        GUI_DEL_OBJ(g_coinListCont)
    }
    CloseToTargetTileView(g_connectWalletTileView.currentTile,
                          CONNECT_WALLET_SELECT_WALLET);
    GuiPendingHintBoxRemove();
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

static void PrivateModePrevTileHandler(lv_event_t *e)
{
    g_isNeedCloseTimer = true;
    CloseToTargetTileView(g_connectWalletTileView.currentTile,
                          CONNECT_WALLET_SELECT_WALLET);
}

static void PrivateModeQRSharingHandler(lv_event_t *e)
{
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    GUI_DEL_OBJ(g_openMoreHintBox);
}

static void ExitPrivateMode(void)
{
    ClosePrivateQrMode();
    RestorePrivateMode();
    g_privateModePincode = NULL;
}

static void RestorePrivateMode(void)
{
    lv_label_set_text(g_connectWalletTileView.privateModePincode, HIDDEN_PINCODE);
    lv_obj_set_style_text_color(g_connectWalletTileView.privateModePincode, lv_color_hex(16090890), LV_PART_MAIN);
    lv_img_set_src(g_connectWalletTileView.privateModePincodeBtn, &imgEye);
    lv_obj_add_flag(g_connectWalletTileView.privateModeQrCodeMask, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_connectWalletTileView.privateModeQrCode, LV_OBJ_FLAG_HIDDEN);
}

static void CloseAttentionHandler(lv_event_t *e)
{
    // ConnectWalletReturnHandler
    if (g_privateModeHintBox) {
        lv_obj_add_flag(g_privateModeHintBox, LV_OBJ_FLAG_HIDDEN);
        GUI_DEL_OBJ(g_privateModeHintBox);
        g_privateModeHintBox = NULL;
    }
}

static void CancelAttentionHandler(lv_event_t *e)
{
    ConnectWalletReturnHandler(NULL);
    CloseAttentionHandler(NULL);
}

static void ContinueAttentionHandler(lv_event_t *e)
{
    g_isFirstOpenPrivateMode = false;
    CloseAttentionHandler(NULL);
    ShowOrHiddenPincode(NULL);
}

static void ShowOrHiddenPincode(lv_event_t *e)
{
    if (g_isFirstOpenPrivateMode) {
        g_privateModeHintBox = GuiCreateGeneralHintBox(&imgWarn, _("security_notice_title"), _("private_mode_security_notice1"), _("private_mode_security_notice2"), _("not_now"), WHITE_COLOR_OPA20, _("understand"), ORANGE_COLOR);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_privateModeHintBox);
        lv_obj_add_event_cb(leftBtn, CancelAttentionHandler, LV_EVENT_CLICKED, NULL);
        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_privateModeHintBox);
        lv_obj_add_event_cb(rightBtn, ContinueAttentionHandler, LV_EVENT_CLICKED, NULL);
        return;
    }
    char *text = lv_label_get_text(g_connectWalletTileView.privateModePincode);
    if (strcmp(text, HIDDEN_PINCODE) == 0) {
        char pincode[BUFFER_SIZE_32] = {0};
        snprintf_s(pincode, sizeof(pincode), "%d %d %d %d %d %d", g_privateModePincode[0],
                   g_privateModePincode[1], g_privateModePincode[2], g_privateModePincode[3],
                   g_privateModePincode[4], g_privateModePincode[5]);
        lv_label_set_text(g_connectWalletTileView.privateModePincode, pincode);
        lv_obj_set_style_text_color(g_connectWalletTileView.privateModePincode, lv_color_hex(16090890), LV_PART_MAIN);
        lv_img_set_src(g_connectWalletTileView.privateModePincodeBtn, &imgEyeOff);
        lv_obj_add_flag(g_connectWalletTileView.privateModeQrCode, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_connectWalletTileView.privateModeQrCodeMask, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text(g_connectWalletTileView.privateModePincode, HIDDEN_PINCODE);
        lv_obj_set_style_text_color(g_connectWalletTileView.privateModePincode, lv_color_hex(16090890), LV_PART_MAIN);
        lv_img_set_src(g_connectWalletTileView.privateModePincodeBtn, &imgEye);
        lv_obj_add_flag(g_connectWalletTileView.privateModeQrCodeMask, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_connectWalletTileView.privateModeQrCode, LV_OBJ_FLAG_HIDDEN);
    }
}

static bool IsSupportEncryption(void)
{
    switch (g_connectWalletTileView.walletIndex) {
    case WALLET_LIST_CAKE:
        return g_privateModePincode == NULL;
    default:
        return false;
    }
}

static void GuiCreateQrCodePrivateModeWidget(lv_obj_t *parent)
{
    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 408);
    lv_obj_add_flag(qrCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(qrCont, 336, 336);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_color(qrBgCont, WHITE_COLOR, LV_PART_MAIN);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(qrBgCont, 294, 294);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);
    lv_obj_t *img = GuiCreateImg(qrBgCont, &imgQrcodeMask);
    lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
    g_connectWalletTileView.privateModeQrCode = qrcode;
    g_connectWalletTileView.privateModeQrCodeMask = img;

    lv_obj_t *pincodeCont = GuiCreateContainerWithParent(parent, 408, 80);
    lv_obj_set_style_bg_color(pincodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(pincodeCont, 24, LV_PART_MAIN);
    lv_obj_align(pincodeCont, LV_ALIGN_TOP_MID, 0, 448);

    lv_obj_t *hidePincode = GuiCreateTitleLabel(pincodeCont, HIDDEN_PINCODE);
    lv_obj_align(hidePincode, LV_ALIGN_TOP_LEFT, 86, 16);
    lv_obj_set_style_text_color(hidePincode, lv_color_hex(16090890), LV_PART_MAIN);
    g_connectWalletTileView.privateModePincode = hidePincode;
    img = GuiCreateImg(pincodeCont, &imgEye);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 290, 22);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, ShowOrHiddenPincode, LV_EVENT_CLICKED, NULL);
    g_connectWalletTileView.privateModePincodeBtn = img;

    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_private_mode_hint"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 550);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
}
