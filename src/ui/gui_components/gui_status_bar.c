#include "gui_status_bar.h"
#include "account_manager.h"
#include "gui_button.h"
#include "gui_chain.h"
#include "gui_connect_wallet_widgets.h"
#include "gui_firmware_update_widgets.h"
#include "gui_home_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_model.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "keystore.h"
#include "usb_task.h"
#include "user_memory.h"
#include "version.h"
#include "gui_firmware_update_widgets.h"

#ifndef COMPILE_SIMULATOR
#include "drv_usb.h"
#include "user_fatfs.h"
#else
#include "simulator_model.h"
static void SwitchWalletHandler(lv_event_t *e)
{
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
}
#endif

typedef struct StatusBar {
    lv_obj_t *background;
#if (WALLPAPER_ENABLE == 1)
    lv_obj_t *wallPaper;
#endif
    lv_obj_t *cont;
    lv_obj_t *walletIcon;
    lv_obj_t *walletNameLabel;
    lv_obj_t *batteryImg;
    lv_obj_t *sdCardImg;
    lv_obj_t *usbImg;
    lv_obj_t *batteryPad;
    lv_obj_t *batteryCharging;
    lv_obj_t *batteryPadImg;
    lv_obj_t *batteryLabel;
    lv_obj_t *betaImg;
#ifdef BTC_ONLY
    lv_obj_t *testNetImg;
#endif
} StatusBar_t;
static StatusBar_t g_guiStatusBar;

typedef struct {
    GuiChainCoinType index;
    const char *name;
    const lv_img_dsc_t *icon;
} CoinWalletInfo_t;

bool GetLvglHandlerStatus(void);

static void RefreshStatusBar(void);

const static CoinWalletInfo_t g_coinWalletBtn[] = {
    {CHAIN_BTC, "", &coinBtc},
#ifndef BTC_ONLY
    {CHAIN_ETH, "", &coinEth},       {CHAIN_SOL, "", &coinSol},
    {CHAIN_BNB, "", &coinBnb},       {CHAIN_XRP, "", &coinXrp},
    {CHAIN_ADA, "", &coinAda},       {CHAIN_DOT, "", &coinDot},
    {CHAIN_TRX, "", &coinTrx},       {CHAIN_LTC, "", &coinLtc},
    {CHAIN_BCH, "", &coinBch},       {CHAIN_APT, "", &coinApt},
    {CHAIN_SUI, "", &coinSui},       {CHAIN_DASH, "", &coinDash},
    {CHAIN_ARWEAVE, "", &coinAr},
    {CHAIN_COSMOS, "", &coinCosmos}, {CHAIN_TIA, "", &coinTia},
    {CHAIN_DYM, "", &coinDym},       {CHAIN_OSMO, "", &coinOsmo},
    {CHAIN_INJ, "", &coinInj},       {CHAIN_ATOM, "", &coinAtom},
    {CHAIN_CRO, "", &coinCro},       {CHAIN_KAVA, "", &coinKava},
    {CHAIN_LUNC, "", &coinLunc},     {CHAIN_AXL, "", &coinAxl},
    {CHAIN_LUNA, "", &coinLuna},     {CHAIN_AKT, "", &coinAkt},
    {CHAIN_STRD, "", &coinStrd},     {CHAIN_SCRT, "", &coinScrt},
    {CHAIN_BLD, "", &coinBld},       {CHAIN_CTK, "", &coinCtk},
    {CHAIN_EVMOS, "", &coinEvmos},   {CHAIN_STARS, "", &coinStars},
    {CHAIN_XPRT, "", &coinXprt},     {CHAIN_SOMM, "", &coinSomm},
    {CHAIN_JUNO, "", &coinJuno},     {CHAIN_IRIS, "", &coinIris},
    {CHAIN_DVPN, "", &coinDvpn},     {CHAIN_ROWAN, "", &coinRowan},
    {CHAIN_REGEN, "", &coinRegen},   {CHAIN_BOOT, "", &coinBoot},
    {CHAIN_GRAV, "", &coinGrav},     {CHAIN_IXO, "", &coinIxo},
    {CHAIN_NGM, "", &coinNgm},       {CHAIN_IOV, "", &coinIov},
    {CHAIN_UMEE, "", &coinUmee},     {CHAIN_QCK, "", &coinQck},
    {CHAIN_TGD, "", &coinTgd},       {CHAIN_TON, "", &coinTon},
#endif
};

const static CoinWalletInfo_t g_walletBtn[] = {
#ifndef BTC_ONLY
    {WALLET_LIST_METAMASK, "MetaMask", &walletMetamask},
    {WALLET_LIST_OKX, "OKX Wallet", &walletOkx},
    {WALLET_LIST_ETERNL, "Eternl Wallet", &walletEternl},
    // {WALLET_LIST_YOROI, "Yoroi Wallet", &walletYoroi},
    {WALLET_LIST_TYPHON, "Typhon Wallet", &walletTyphon},
    {WALLET_LIST_BLUE, "BlueWallet", &walletBluewallet},
    {WALLET_LIST_SUB, "SubWallet", &walletSubwallet},
    {WALLET_LIST_SOLFARE, "Solflare", &walletSolflare},
    {WALLET_LIST_RABBY, "Rabby", &walletRabby},
    {WALLET_LIST_SAFE, "Safe", &walletSafe},
    {WALLET_LIST_SPARROW, "Sparrow", &walletSparrow},
    {WALLET_LIST_UNISAT, "UniSat", &walletUniSat},
    {WALLET_LIST_IMTOKEN, "imToken", &walletImToken},
    {WALLET_LIST_BLOCK_WALLET, "Block Wallet", &walletBlockWallet},
    {WALLET_LIST_ZAPPER, "Zapper", &walletZapper},
    {WALLET_LIST_YEARN_FINANCE, "Yearn Finance", &walletYearn},
    {WALLET_LIST_SUSHISWAP, "SushiSwap", &walletSushi},
    {WALLET_LIST_KEPLR, "Keplr", &walletKeplr},
    {WALLET_LIST_ARCONNECT, "ArConnect", &walletArConnect},
    {WALLET_LIST_FEWCHA, "Fewcha", &walletFewcha},
    {WALLET_LIST_PETRA, "Petra", &walletPetra},
    {WALLET_LIST_XRP_TOOLKIT, "XRP Toolkit", &walletXRPToolkit},
    {WALLET_LIST_TONKEEPER, "Tonkeeper", &walletTonkeeper},
#else
    {WALLET_LIST_BLUE, "BlueWallet", &walletBluewallet},
    {WALLET_LIST_SPECTER, "Specter", &walletSpecter},
    {WALLET_LIST_SPARROW, "Sparrow", &walletSparrow},
    {WALLET_LIST_NUNCHUK, "Nunchuk", &walletNunchuk},
    {WALLET_LIST_UNISAT, "UniSat", &walletUniSat},
#endif
};

void GuiNvsBarSetWalletName(const char *name)
{
    lv_label_set_text(g_guiStatusBar.walletNameLabel, name);
}

const char *GuiNvsBarGetWalletName(void)
{
    return lv_label_get_text(g_guiStatusBar.walletNameLabel);
}

void GuiNvsBarSetWalletIcon(const void *src)
{
    if (g_guiStatusBar.walletIcon != NULL) {
        lv_obj_del(g_guiStatusBar.walletIcon);
        g_guiStatusBar.walletIcon = NULL;
    }
    if (src == NULL) {
        return;
    }
    g_guiStatusBar.walletIcon = lv_img_create(g_guiStatusBar.cont);
    lv_img_set_src(g_guiStatusBar.walletIcon, src);
    lv_img_set_zoom(g_guiStatusBar.walletIcon, 200);
    lv_obj_align(g_guiStatusBar.walletIcon, LV_ALIGN_LEFT_MID, 26, 0);
}

void ShowWallPaper(bool enable)
{
#if (WALLPAPER_ENABLE == 1)
    if (enable) {
        lv_obj_clear_flag(g_guiStatusBar.wallPaper, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_guiStatusBar.wallPaper, LV_OBJ_FLAG_HIDDEN);
    }
#endif
}

void GuiStatusBarInit(void)
{
    g_guiStatusBar.background = GuiCreateContainer(
                                    lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
#if (WALLPAPER_ENABLE == 1)
    g_guiStatusBar.wallPaper = GuiCreateImg(g_guiStatusBar.background, NULL);
    lv_img_set_src(g_guiStatusBar.wallPaper, &imgDeepLayersVolume11);
    ShowWallPaper(false);
#endif
    lv_obj_t *cont = GuiCreateContainerWithParent(g_guiStatusBar.background,
                     lv_obj_get_width(lv_scr_act()),
                     GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_size(cont, lv_obj_get_width(lv_scr_act()), GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_obj_t *body = GuiCreateContainerWithParent(
                         g_guiStatusBar.background, lv_obj_get_width(lv_scr_act()),
                         lv_obj_get_height(lv_scr_act()) - GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_radius(body, 0, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 0, GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, LV_PART_MAIN);

    g_guiStatusBar.cont = cont;
    lv_obj_t *img = GuiCreateImg(cont, NULL);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 24, 0);
    g_guiStatusBar.walletIcon = img;

    lv_obj_t *label = GuiCreateIllustrateLabel(cont, "");
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 0);
    g_guiStatusBar.walletNameLabel = label;

    img = GuiCreateImg(cont, &imgBattery);
    lv_obj_align(img, LV_ALIGN_RIGHT_MID, -70, 0);
    g_guiStatusBar.batteryImg = img;

    g_guiStatusBar.batteryCharging = GuiCreateImg(cont, &imgCharging);
    lv_obj_align(g_guiStatusBar.batteryCharging, LV_ALIGN_RIGHT_MID, -70, 0);
    lv_obj_add_flag(g_guiStatusBar.batteryCharging, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_img_opa(g_guiStatusBar.batteryCharging, LV_OPA_COVER,
                             LV_PART_MAIN);
    lv_obj_set_style_opa(g_guiStatusBar.batteryCharging, LV_OPA_COVER,
                         LV_PART_MAIN);

    g_guiStatusBar.batteryPad = lv_obj_create(g_guiStatusBar.batteryImg);
    lv_obj_align(g_guiStatusBar.batteryPad, LV_ALIGN_TOP_LEFT, 6, 7);
    // lv_obj_set_pos(g_guiStatusBar.batteryPad, 6, 7);
    lv_obj_set_size(g_guiStatusBar.batteryPad, 0, 10);
    lv_obj_set_style_outline_width(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_set_style_outline_pad(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_set_style_border_width(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_set_style_radius(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_clear_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_guiStatusBar.batteryPad,
                              lv_color_make(0xFF, 0xFF, 0xFF), 0);
    g_guiStatusBar.batteryPadImg = lv_img_create(g_guiStatusBar.batteryImg);
    lv_obj_set_pos(g_guiStatusBar.batteryPadImg, 6, 7);

    label = GuiCreateIllustrateLabel(cont, " ");
    lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 414, 0);
    g_guiStatusBar.batteryLabel = label;

    img = GuiCreateImg(cont, &imgSdCard);
    g_guiStatusBar.sdCardImg = img;
    if (!SdCardInsert()) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
    }

    img = GuiCreateImg(cont, &imgUsb);
    g_guiStatusBar.usbImg = img;
    lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);

    if (SOFTWARE_VERSION_BUILD % 2) {
        img = GuiCreateImg(cont, &imgBeta);
        g_guiStatusBar.betaImg = img;
    }
#ifdef BTC_ONLY
    img = GuiCreateImg(cont, &imgTestNet);
    g_guiStatusBar.testNetImg = img;
    lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
#endif
    RefreshStatusBar();
#ifdef COMPILE_SIMULATOR
    GuiStatusBarSetBattery(88, true);
    lv_obj_t *btn = GuiCreateTextBtn(cont, "switch");
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
    lv_obj_add_event_cb(btn, SwitchWalletHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 0);
#endif
}

void GuiStatusBarSetSdCard(bool connected, bool onlyImg)
{
    if (!GetLvglHandlerStatus()) {
        return;
    }
    static int32_t sdStatus = -1;
    if (sdStatus == (connected ? 1 : 0)) {
        return;
    }
    sdStatus = connected;
    if (connected) {
        lv_obj_clear_flag(g_guiStatusBar.sdCardImg, LV_OBJ_FLAG_HIDDEN);
        if (!onlyImg) {
            uint8_t accountCnt = 0;
            GetExistAccountNum(&accountCnt);
            if (!GuiLockScreenIsTop() && accountCnt > 0 && FatfsFileExist(SD_CARD_OTA_BIN_PATH) && !GuiCheckIfTopView(&g_forgetPassView)) {
                GuiCreateSdCardUpdateHintbox(false);
            }
        }
    } else {
        lv_obj_add_flag(g_guiStatusBar.sdCardImg, LV_OBJ_FLAG_HIDDEN);
    }
    RefreshStatusBar();
}

void GuiStatusBarSetUsb(void)
{
    if (GetUsbState() && UsbInitState()) {
        lv_obj_clear_flag(g_guiStatusBar.usbImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_guiStatusBar.usbImg, LV_OBJ_FLAG_HIDDEN);
    }
    RefreshStatusBar();
}

#ifdef BTC_ONLY
void GuiStatusBarSetTestNet(void)
{
    if ((GetIsTestNet() == true) && (GetCurrentWalletIndex() == SINGLE_WALLET) &&
            GetCurrentAccountIndex() < 3) {
        lv_obj_clear_flag(g_guiStatusBar.testNetImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_guiStatusBar.testNetImg, LV_OBJ_FLAG_HIDDEN);
    }
    RefreshStatusBar();
}
#else
char *GetWalletNameByIndex(WALLET_LIST_INDEX_ENUM index)
{
    if (index == WALLET_LIST_ETERNL) {
        return "Eternl";
    } else if (index == WALLET_LIST_TYPHON) {
        return "Typhon";
    }
    return g_walletBtn[index].name;
}
#endif

void GuiStatusBarSetBattery(uint8_t percent, bool charging)
{
    char percentStr[BUFFER_SIZE_16];

    snprintf_s(percentStr, BUFFER_SIZE_16, "%d%%", percent);
    lv_label_set_text(g_guiStatusBar.batteryLabel, percentStr);

    if (charging) {
        lv_obj_align(g_guiStatusBar.batteryImg, LV_ALIGN_RIGHT_MID, -89, 0);
        lv_obj_clear_flag(g_guiStatusBar.batteryCharging, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_align(g_guiStatusBar.batteryImg, LV_ALIGN_RIGHT_MID, -70, 0);
        lv_obj_add_flag(g_guiStatusBar.batteryCharging, LV_OBJ_FLAG_HIDDEN);
    }
    if (percent == 100 && charging) {
        lv_obj_add_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(g_guiStatusBar.batteryPadImg, &imgBatteryPowerFull);
        lv_obj_clear_flag(g_guiStatusBar.batteryPadImg, LV_OBJ_FLAG_HIDDEN);
    } else if (percent == 0) {
        lv_obj_add_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_guiStatusBar.batteryPadImg, LV_OBJ_FLAG_HIDDEN);
    } else if (percent < 10) {
        lv_obj_add_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(g_guiStatusBar.batteryPadImg, &imgBatteryPower10);
        lv_obj_clear_flag(g_guiStatusBar.batteryPadImg, LV_OBJ_FLAG_HIDDEN);
    } else if (percent < 20) {
        lv_obj_add_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(g_guiStatusBar.batteryPadImg, &imgBatteryPower20);
        lv_obj_clear_flag(g_guiStatusBar.batteryPadImg, LV_OBJ_FLAG_HIDDEN);
    } else if (percent < 30) {
        lv_obj_add_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(g_guiStatusBar.batteryPadImg, &imgBatteryPower30);
        lv_obj_clear_flag(g_guiStatusBar.batteryPadImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_guiStatusBar.batteryPadImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(g_guiStatusBar.batteryPad, percent / 10 * 2, 10);
    }
    RefreshStatusBar();
}

static void RefreshStatusBar(void)
{
    lv_obj_t *next;
    next = g_guiStatusBar.batteryImg;
    lv_obj_align_to(g_guiStatusBar.sdCardImg, next, LV_ALIGN_OUT_LEFT_MID, -10,
                    0);
    if (!lv_obj_has_flag(g_guiStatusBar.sdCardImg, LV_OBJ_FLAG_HIDDEN)) {
        next = g_guiStatusBar.sdCardImg;
    }
    lv_obj_align_to(g_guiStatusBar.usbImg, next, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    if (!lv_obj_has_flag(g_guiStatusBar.usbImg, LV_OBJ_FLAG_HIDDEN)) {
        next = g_guiStatusBar.usbImg;
    }
#ifdef BTC_ONLY
    lv_obj_align_to(g_guiStatusBar.testNetImg, next, LV_ALIGN_OUT_LEFT_MID, -10,
                    0);
    next = g_guiStatusBar.testNetImg;
#endif
    if (SOFTWARE_VERSION_BUILD % 2) {
        lv_obj_align_to(g_guiStatusBar.betaImg, next, LV_ALIGN_OUT_LEFT_MID, -10,
                        0);
    }
}

static lv_obj_t *CreateReturnBtn(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgArrowLeft, 64, NULL, NULL);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    return btn;
}

static lv_obj_t *CreateCloseBtn(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgClose, 64, NULL, NULL);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    return btn;
}

static lv_obj_t *CreateManageBtn(lv_obj_t *navBar)
{
#ifdef BTC_ONLY
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgWallet2, 64, NULL, NULL);
#else
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgManage, 64, NULL, NULL);
#endif
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    return btn;
}

static lv_obj_t *CreateMidLabel(lv_obj_t *navBar)
{
    lv_obj_t *label = GuiCreateTextLabel(navBar, "");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    return label;
}

static lv_obj_t *CreateMidWordCntSelect(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateBtnWithFont(navBar, "20    " USR_SYMBOL_DOWN,
                                         g_defIllustrateFont);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 268, 0);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_size(btn, 69, 42);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);

    return btn;
}

static lv_obj_t *CreateCoinBtn(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateStatusCoinButton(navBar, _(""), &walletBluewallet);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);

    return btn;
}

static lv_obj_t *CreateWordCntSelect(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateLabelImgAdaptButton(navBar, _("24"), &imgArrowDownS, NULL, NULL);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_size(btn, 69, 42);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);

    return btn;
}

static lv_obj_t *CreateResetButton(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgLabelAdaptButton(navBar, _("single_phrase_reset"),
                    &imgReset, NULL, NULL);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_size(btn, 106, 42);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateQuestion(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgQuestion, 64, NULL, NULL);
    lv_obj_set_size(btn, 106, 42);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -24, 0);
    return btn;
}

static lv_obj_t *CreateMoreInfo(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgMore, 64, NULL, NULL);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    return btn;
}

static lv_obj_t *CreateSkip(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgSkip, 64, NULL, NULL);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    return btn;
}

static lv_obj_t *CreateSearch(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgSearch, 64, NULL, NULL);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    return btn;
}

static lv_obj_t *CreateNewSkip(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateTextBtn(navBar, _("Skip"));
    lv_obj_set_style_bg_color(btn, GRAY_COLOR, 0);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_set_height(btn, 42);
    return btn;
}

static lv_obj_t *CreateUndo(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgLabelAdaptButton(navBar, _("Undo"), &imgUndo, NULL, NULL);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, GRAY_COLOR, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_10 + LV_OPA_2, LV_STATE_PRESSED | LV_PART_MAIN);
    return btn;
}

static lv_obj_t *CreateSDCard(lv_obj_t *navBar)
{
    lv_obj_t *btn = GuiCreateImgButton(navBar, &imgSdCardColor, 64, NULL, NULL);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    return btn;
}

NavBarWidget_t *CreateNavBarWidget(lv_obj_t *navBar)
{

    NavBarWidget_t *navBarWidget = SRAM_MALLOC(sizeof(NavBarWidget_t));

    navBarWidget->navBar = navBar;
    navBarWidget->leftBtn = NULL;
    navBarWidget->midBtn = NULL;
    navBarWidget->rightBtn = NULL;

    return navBarWidget;
}

void DestoryNavBarWidget(NavBarWidget_t *navBarWidget)
{
    if (navBarWidget != NULL) {
        if (navBarWidget->navBar != NULL && lv_obj_is_valid(navBarWidget->navBar)) {
            lv_obj_del(navBarWidget->navBar);
        }

        SRAM_FREE(navBarWidget);
    }
}

void SetNavBarLeftBtn(NavBarWidget_t *navBarWidget, NVS_LEFT_BUTTON_ENUM button,
                      lv_event_cb_t eventCb, void *param)
{
    if (navBarWidget->leftBtn != NULL && lv_obj_is_valid(navBarWidget->leftBtn)) {
        lv_obj_del(navBarWidget->leftBtn);
        navBarWidget->leftBtn = NULL;
    }
    lv_event_cb_t leftButtonCb = NULL;
    switch (button) {
    case NVS_BAR_RETURN:
        navBarWidget->leftBtn = CreateReturnBtn(navBarWidget->navBar);
        leftButtonCb = eventCb;
        break;
    case NVS_BAR_CLOSE:
        navBarWidget->leftBtn = CreateCloseBtn(navBarWidget->navBar);
        leftButtonCb = eventCb;
        break;
    case NVS_BAR_MANAGE:
        navBarWidget->leftBtn = CreateManageBtn(navBarWidget->navBar);
        leftButtonCb = eventCb;
        break;
    default:
        return;
    }
    lv_obj_clear_flag(navBarWidget->leftBtn, LV_OBJ_FLAG_HIDDEN);
    if (leftButtonCb != NULL) {
        if (button != NVS_BAR_MANAGE) {
            lv_obj_add_event_cb(navBarWidget->leftBtn, leftButtonCb, LV_EVENT_CLICKED,
                                param);
        } else {
            lv_obj_add_event_cb(navBarWidget->leftBtn, leftButtonCb, LV_EVENT_CLICKED,
                                param);
        }
    }
}

void SetNavBarMidBtn(NavBarWidget_t *navBarWidget, NVS_MID_BUTTON_ENUM button,
                     lv_event_cb_t eventCb, void *param)
{
    if (navBarWidget->midBtn != NULL && lv_obj_is_valid(navBarWidget->midBtn)) {
        lv_obj_del(navBarWidget->midBtn);
        navBarWidget->midBtn = NULL;
    }
    lv_event_cb_t midButtonCb = NULL;
    switch (button) {
    case NVS_BAR_MID_WORD_SELECT:
        navBarWidget->midBtn = CreateMidWordCntSelect(navBarWidget->navBar);
        midButtonCb = eventCb;
        break;
    case NVS_BAR_MID_LABEL:
        navBarWidget->midBtn = CreateMidLabel(navBarWidget->navBar);
        midButtonCb = eventCb;
        break;
    case NVS_BAR_MID_COIN:
        navBarWidget->midBtn = CreateCoinBtn(navBarWidget->navBar);
        return;
    default:
        return;
    }
    lv_obj_clear_flag(navBarWidget->midBtn, LV_OBJ_FLAG_HIDDEN);
    if (midButtonCb != NULL) {
        lv_obj_add_event_cb(navBarWidget->midBtn, midButtonCb, LV_EVENT_CLICKED,
                            param);
    }
}

void SetCoinWallet(NavBarWidget_t *navBarWidget, GuiChainCoinType index,
                   const char *name)
{
    SetNavBarMidBtn(navBarWidget, NVS_BAR_MID_COIN, NULL, NULL);
    navBarWidget->midBtn = GuiUpdateStatusCoinButton(
                               navBarWidget->midBtn, (name != NULL) ? name : _("confirm_transaction"),
                               g_coinWalletBtn[index].icon);
}

void SetWallet(NavBarWidget_t *navBarWidget, WALLET_LIST_INDEX_ENUM index,
               const char *name)
{
    SetNavBarMidBtn(navBarWidget, NVS_BAR_MID_COIN, NULL, NULL);
    if (name == NULL) {
        char name[BUFFER_SIZE_64] = {0};
        snprintf_s(name, BUFFER_SIZE_64, "%s %s", _("connect_head"),
                   g_walletBtn[index].name);
        navBarWidget->midBtn = GuiUpdateStatusCoinButton(navBarWidget->midBtn, name,
                               g_walletBtn[index].icon);
    } else {
        navBarWidget->midBtn = GuiUpdateStatusCoinButton(navBarWidget->midBtn, name,
                               g_walletBtn[index].icon);
    }
}

void SetMidBtnLabel(NavBarWidget_t *navBarWidget, NVS_MID_BUTTON_ENUM button,
                    const char *text)
{
    SetNavBarMidBtn(navBarWidget, button, NULL, NULL);
    switch (button) {
    case NVS_BAR_MID_WORD_SELECT:
        lv_label_set_text(lv_obj_get_child(navBarWidget->midBtn, 0), text);
        break;
    case NVS_BAR_MID_LABEL:
        lv_label_set_text(navBarWidget->midBtn, text);
        lv_obj_clear_flag(navBarWidget->midBtn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_refr_size(navBarWidget->midBtn);
        if (lv_obj_get_self_width(navBarWidget->midBtn) > 300) {
            lv_label_set_long_mode(navBarWidget->midBtn, LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_width(navBarWidget->midBtn, 300);
        }

        // GuiNvsBarSetMidCb(NVS_BAR_MID_LABEL, NULL, NULL);
        break;
    default:
        return;
    }
}

void SetRightBtnLabel(NavBarWidget_t *navBarWidget,
                      NVS_RIGHT_BUTTON_ENUM button, const char *text)
{
    SetNavBarRightBtn(navBarWidget, button, NULL, NULL);
    switch (button) {
    case NVS_BAR_WORD_SELECT:
        lv_label_set_text(lv_obj_get_child(navBarWidget->rightBtn, 0), text);
        break;
    case NVS_BAR_WORD_RESET:
        lv_label_set_text(lv_obj_get_child(navBarWidget->rightBtn, 1), text);
        GuiImgLabelAdaptButtonResize(navBarWidget->rightBtn);
        break;
    default:
        return;
    }
}

void SetRightBtnCb(NavBarWidget_t *navBarWidget, lv_event_cb_t eventCb,
                   void *param)
{
    lv_obj_add_event_cb(navBarWidget->rightBtn, eventCb, LV_EVENT_CLICKED, param);
}

void SetNavBarRightBtn(NavBarWidget_t *navBarWidget,
                       NVS_RIGHT_BUTTON_ENUM button, lv_event_cb_t eventCb,
                       void *param)
{
    if (navBarWidget->rightBtn != NULL &&
            lv_obj_is_valid(navBarWidget->rightBtn)) {
        lv_obj_del(navBarWidget->rightBtn);
        navBarWidget->rightBtn = NULL;
    }
    lv_event_cb_t rightButtonCb = NULL;
    switch (button) {
    case NVS_BAR_WORD_SELECT:
        navBarWidget->rightBtn = CreateWordCntSelect(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_WORD_RESET:
        navBarWidget->rightBtn = CreateResetButton(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_QUESTION_MARK:
        navBarWidget->rightBtn = CreateQuestion(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_MORE_INFO:
        navBarWidget->rightBtn = CreateMoreInfo(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_SKIP:
        navBarWidget->rightBtn = CreateSkip(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_SEARCH:
        navBarWidget->rightBtn = CreateSearch(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_NEW_SKIP:
        navBarWidget->rightBtn = CreateNewSkip(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_UNDO:
        navBarWidget->rightBtn = CreateUndo(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_SDCARD:
        navBarWidget->rightBtn = CreateSDCard(navBarWidget->navBar);
        rightButtonCb = eventCb;
        break;
    default:
        return;
    }
    lv_obj_clear_flag(navBarWidget->rightBtn, LV_OBJ_FLAG_HIDDEN);
    if (rightButtonCb != NULL) {
        lv_obj_add_event_cb(navBarWidget->rightBtn, rightButtonCb, LV_EVENT_CLICKED,
                            param);
    }
}
