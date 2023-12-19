#include "gui_obj.h"
#include "gui_views.h"
#include "gui_button.h"
#include "gui_status_bar.h"
#include "gui_firmware_update_widgets.h"
#include "firmware_update.h"
#include "gui_model.h"
#include "gui_lock_widgets.h"
#include "keystore.h"
#include "usb_task.h"
#include "user_memory.h"
#include "account_manager.h"

#ifndef COMPILE_SIMULATOR
#include "user_fatfs.h"
#else
#include "simulator_model.h"
#endif

typedef struct StatusBar {
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
} StatusBar_t;
static StatusBar_t g_guiStatusBar;

typedef struct {
    GuiChainCoinType index;
    const char *name;
    const lv_img_dsc_t *icon;
} CoinWalletInfo_t;

bool GetLvglHandlerStatus(void);

const static CoinWalletInfo_t g_coinWalletBtn[] = {
    {CHAIN_BTC, "Confirm Transaction", &coinBtc},
    {CHAIN_ETH, "Confirm Transaction", &coinEth},
    {CHAIN_SOL, "Confirm Transaction", &coinSol},
    {CHAIN_APT, "Confirm Transaction", &coinApt},
    {CHAIN_SUI, "Confirm Transaction", &coinSui},
    {CHAIN_ADA, "Confirm Transaction", &coinAda},
    {CHAIN_XRP, "Confirm Transaction", &coinXrp},
    {CHAIN_TRX, "Confirm Transaction", &coinTrx},
    {CHAIN_BCH, "Confirm Transaction", &coinBch},
    {CHAIN_DASH, "Confirm Transaction", &coinDash},
    {CHAIN_LTC, "Confirm Transaction", &coinLtc},
    {CHAIN_ATOM, "Confirm Transaction", &coinAtom},
    {CHAIN_OSMO, "Confirm Transaction", &coinOsmo},
    {CHAIN_SCRT, "Confirm Transaction", &coinScrt},
    {CHAIN_AKT, "Confirm Transaction", &coinAkt},
    {CHAIN_CRO, "Confirm Transaction", &coinCro},
    {CHAIN_IOV, "Confirm Transaction", &coinIov},
    {CHAIN_ROWAN, "Confirm Transaction", &coinRowan},
    {CHAIN_CTK, "Confirm Transaction", &coinCtk},
    {CHAIN_IRIS, "Confirm Transaction", &coinIris},
    {CHAIN_REGEN, "Confirm Transaction", &coinRegen},
    {CHAIN_XPRT, "Confirm Transaction", &coinXprt},
    {CHAIN_DVPN, "Confirm Transaction", &coinDvpn},
    {CHAIN_IXO, "Confirm Transaction", &coinIxo},
    {CHAIN_NGM, "Confirm Transaction", &coinNgm},
    {CHAIN_BLD, "Confirm Transaction", &coinBld},
    {CHAIN_BOOT, "Confirm Transaction", &coinBoot},
    {CHAIN_JUNO, "Confirm Transaction", &coinJuno},
    {CHAIN_STARS, "Confirm Transaction", &coinStars},
    {CHAIN_AXL, "Confirm Transaction", &coinAxl},
    {CHAIN_SOMM, "Confirm Transaction", &coinSomm},
    {CHAIN_UMEE, "Confirm Transaction", &coinUmee},
    {CHAIN_GRAV, "Confirm Transaction", &coinGrav},
    {CHAIN_TGD, "Confirm Transaction", &coinTgd},
    {CHAIN_STRD, "Confirm Transaction", &coinStrd},
    {CHAIN_EVMOS, "Confirm Transaction", &coinEvmos},
    {CHAIN_INJ, "Confirm Transaction", &coinInj},
    {CHAIN_KAVA, "Confirm Transaction", &coinKava},
    {CHAIN_QCK, "Confirm Transaction", &coinQck},
    {CHAIN_LUNA, "Confirm Transaction", &coinLuna},
    {CHAIN_LUNC, "Confirm Transaction", &coinLunc},
    {CHAIN_BNB, "Confirm Transaction", &coinBnb},
    {CHAIN_DOT, "Confirm Transaction", &coinDot},
};

const static CoinWalletInfo_t g_walletBtn[] = {
    {WALLET_LIST_KEYSTONE, "Connect Keystone Wallet", &walletKeystone},
    {WALLET_LIST_METAMASK, "Connect MetaMask", &walletMetamask},
    {WALLET_LIST_OKX, "Connect OKX Wallet", &walletOkx},
    {WALLET_LIST_ETERNL, "Connect Eternl Wallet", &walletEternl},
    {WALLET_LIST_BLUE, "Connect BlueWallet", &walletBluewallet},
    {WALLET_LIST_SUB, "Connect SubWallet", &walletSubwallet},
    {WALLET_LIST_SOLFARE, "Connect Solflare", &walletSolflare},
    {WALLET_LIST_RABBY, "Connect Rabby", &walletRabby},
    {WALLET_LIST_SAFE, "Connect Safe", &walletSafe},
    {WALLET_LIST_SPARROW, "Connect Sparrow", &walletSparrow},
    {WALLET_LIST_IMTOKEN, "Connect imToken", &walletImToken},
    {WALLET_LIST_BLOCK_WALLET, "Connect Block Wallet", &walletBlockWallet},
    {WALLET_LIST_ZAPPER, "Connect Zapper", &walletZapper},
    {WALLET_LIST_YEARN_FINANCE, "Connect Yearn Finance", &walletYearn},
    {WALLET_LIST_SUSHISWAP, "Connect SushiSwap", &walletSushi},
    {WALLET_LIST_KEPLR, "Connect Keplr", &walletKeplr},
    {WALLET_LIST_FEWCHA, "Connect Fewcha", &walletFewcha},
    {WALLET_LIST_PETRA, "Connect Petra", &walletPetra},
    {WALLET_LIST_XRP_TOOLKIT, "Connect XRP Toolkit", &walletXRPToolkit},
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

void GuiStatusBarInit(void)
{
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_size(cont, lv_obj_get_width(lv_scr_act()), GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_radius(cont, 0, 0);

    lv_obj_t *background = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_radius(background, 0, 0);
    lv_obj_align(background, LV_ALIGN_TOP_LEFT, 0, GUI_STATUS_BAR_HEIGHT);

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

    g_guiStatusBar.batteryPad = lv_obj_create(g_guiStatusBar.batteryImg);
    lv_obj_align(g_guiStatusBar.batteryPad, LV_ALIGN_TOP_LEFT, 6, 7);
    // lv_obj_set_pos(g_guiStatusBar.batteryPad, 6, 7);
    lv_obj_set_size(g_guiStatusBar.batteryPad, 0, 10);
    lv_obj_set_style_outline_width(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_set_style_outline_pad(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_set_style_border_width(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_set_style_radius(g_guiStatusBar.batteryPad, 0, 0);
    lv_obj_clear_flag(g_guiStatusBar.batteryPad, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_guiStatusBar.batteryPad, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    g_guiStatusBar.batteryPadImg = lv_img_create(g_guiStatusBar.batteryImg);
    lv_obj_set_pos(g_guiStatusBar.batteryPadImg, 6, 7);

    label = GuiCreateIllustrateLabel(cont, " ");
    lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 414, 0);
    g_guiStatusBar.batteryLabel = label;

    img = GuiCreateImg(cont, &imgSdCard);
    g_guiStatusBar.sdCardImg = img;
    lv_obj_align_to(g_guiStatusBar.sdCardImg, g_guiStatusBar.batteryImg, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    if (!SdCardInsert()) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
    }

    img = GuiCreateImg(cont, &imgUsb);
    g_guiStatusBar.usbImg = img;
    lv_obj_align_to(g_guiStatusBar.usbImg, g_guiStatusBar.sdCardImg, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
#ifdef COMPILE_SIMULATOR
    GuiStatusBarSetBattery(88, true);
#endif
}

void GuiStatusBarSetSdCard(bool connected)
{
    if (!GetLvglHandlerStatus()) {
        return;
    }
    static bool sdStatus = true;
    char version[16] = {0};
    if (sdStatus == connected) {
        return;
    }
    sdStatus = connected;
    if (connected) {
        lv_obj_clear_flag(g_guiStatusBar.sdCardImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(g_guiStatusBar.sdCardImg, g_guiStatusBar.batteryImg, LV_ALIGN_OUT_LEFT_MID, -10, 0);
        uint8_t accountCnt = 0;
        GetExistAccountNum(&accountCnt);
        if (!GuiLockScreenIsTop() && CheckOtaBinVersion(version) && accountCnt > 0 && !GuiCheckIfTopView(&g_forgetPassView)) {
            GuiCreateSdCardUpdateHintbox(version, false);
        }
    } else {
        lv_obj_add_flag(g_guiStatusBar.sdCardImg, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiStatusBarSetUsb()
{
    if (GetUsbState()) {
        lv_obj_clear_flag(g_guiStatusBar.usbImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_guiStatusBar.usbImg, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiStatusBarSetBattery(uint8_t percent, bool charging)
{
    char percentStr[16];

    sprintf(percentStr, "%d%%", percent);
    lv_label_set_text(g_guiStatusBar.batteryLabel, percentStr);

    if (charging) {
        lv_obj_align(g_guiStatusBar.batteryImg, LV_ALIGN_RIGHT_MID, -89, 0);
        lv_obj_align_to(g_guiStatusBar.sdCardImg, g_guiStatusBar.batteryImg, LV_ALIGN_OUT_LEFT_MID, -10, 0);
        lv_obj_align_to(g_guiStatusBar.usbImg, g_guiStatusBar.sdCardImg, LV_ALIGN_OUT_LEFT_MID, -10, 0);
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
}


static lv_obj_t *CreateReturnBtn(lv_obj_t *navBar)
{
    lv_obj_t *btn, *img;

    btn = GuiCreateBtn(navBar, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    img = GuiCreateImg(btn, &imgArrowLeft);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateCloseBtn(lv_obj_t *navBar)
{
    lv_obj_t *btn, *img;

    btn = GuiCreateBtn(navBar, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    img = GuiCreateImg(btn, &imgClose);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateManageBtn(lv_obj_t *navBar)
{
    lv_obj_t *btn, *img;

    btn = GuiCreateBtn(navBar, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    img = GuiCreateImg(btn, &imgManage);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateMidLabel(lv_obj_t *navBar)
{
    lv_obj_t *label;

    label = GuiCreateTextLabel(navBar, "");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    return label;
}

static lv_obj_t *CreateMidWordCntSelect(lv_obj_t *navBar)
{
    lv_obj_t *btn;

    btn = GuiCreateBtnWithFont(navBar, "20    " USR_SYMBOL_DOWN, &openSans_20);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 268, 0);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_size(btn, 69, 42);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);

    return btn;
}

static lv_obj_t *CreateCoinBtn(lv_obj_t *navBar)
{
    lv_obj_t *btn;

    btn = GuiCreateStatusCoinButton(navBar, _("Connect BlueWallet"), &walletBluewallet);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);

    return btn;
}

static lv_obj_t *CreateWordCntSelect(lv_obj_t *navBar)
{
    lv_obj_t *btn;

    btn = GuiCreateBtnWithFont(navBar, "24    " USR_SYMBOL_DOWN, &openSans_20);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 387, 0);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_size(btn, 69, 42);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);

    return btn;
}

static lv_obj_t *CreateResetButton(lv_obj_t *navBar)
{
    lv_obj_t *btn;

    btn = GuiCreateBtnWithFont(navBar, _("single_phrase_reset"), &openSansEnIllustrate);
    lv_obj_set_size(btn, 106, 42);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 350, 27);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateQuestion(lv_obj_t *navBar)
{
    lv_obj_t *btn, *img;

    btn = GuiCreateBtn(navBar, "");
    img = GuiCreateImg(btn, &imgQuestion);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_size(btn, 106, 42);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 380, 27);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateMoreInfo(lv_obj_t *navBar)
{
    lv_obj_t *btn, *img;

    btn = GuiCreateBtn(navBar, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    img = GuiCreateImg(btn, &imgMore);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateSkip(lv_obj_t *navBar)
{
    lv_obj_t *btn, *img;

    btn = GuiCreateBtn(navBar, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    img = GuiCreateImg(btn, &imgSkip);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateSearch(lv_obj_t *navBar)
{
    lv_obj_t *btn, *img;

    btn = GuiCreateBtn(navBar, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    img = GuiCreateImg(btn, &imgSearch);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);

    return btn;
}

static lv_obj_t *CreateNewSkip(lv_obj_t *navBar)
{
    lv_obj_t *btn, *textLabel;

    btn = lv_label_create(navBar);
    lv_label_set_text(btn, "");
    lv_obj_set_size(btn, 63, 42);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, GRAY_COLOR, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_10 + LV_OPA_2, LV_STATE_PRESSED | LV_PART_MAIN);

    textLabel = lv_label_create(btn);
    lv_label_set_text(textLabel, "Skip");
    lv_obj_set_style_text_font(textLabel, &openSans_20, LV_PART_MAIN);
    lv_obj_set_style_text_opa(textLabel, LV_OPA_90, LV_PART_MAIN);
    lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_bg_opa(textLabel, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_align(textLabel, LV_ALIGN_CENTER);
    lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 2);
    lv_obj_set_style_text_color(textLabel, WHITE_COLOR, LV_PART_MAIN);

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


void SetNavBarLeftBtn(NavBarWidget_t *navBarWidget, NVS_LEFT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param)
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
            lv_obj_add_event_cb(navBarWidget->leftBtn, leftButtonCb, LV_EVENT_CLICKED, param);
        } else {
            lv_obj_add_event_cb(navBarWidget->leftBtn, leftButtonCb, LV_EVENT_CLICKED, param);
        }
    }
}


void SetNavBarMidBtn(NavBarWidget_t *navBarWidget, NVS_MID_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param)
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
        lv_obj_add_event_cb(navBarWidget->midBtn, midButtonCb, LV_EVENT_CLICKED, param);
    }
}


void SetCoinWallet(NavBarWidget_t *navBarWidget, GuiChainCoinType index, const char *name)
{
    SetNavBarMidBtn(navBarWidget, NVS_BAR_MID_COIN, NULL, NULL);
    navBarWidget->midBtn = GuiUpdateStatusCoinButton(navBarWidget->midBtn, (name != NULL) ? name : g_coinWalletBtn[index].name,
                           g_coinWalletBtn[index].icon);
}

void SetWallet(NavBarWidget_t *navBarWidget, WALLET_LIST_INDEX_ENUM index, const char *name)
{
    SetNavBarMidBtn(navBarWidget, NVS_BAR_MID_COIN, NULL, NULL);
    if (name == NULL) {
        navBarWidget->midBtn = GuiUpdateStatusCoinButton(navBarWidget->midBtn, g_walletBtn[index].name,
                               g_walletBtn[index].icon);
    } else {
        navBarWidget->midBtn = GuiUpdateStatusCoinButton(navBarWidget->midBtn, name,
                               g_walletBtn[index].icon);
    }
}

void SetMidBtnLabel(NavBarWidget_t *navBarWidget, NVS_MID_BUTTON_ENUM button, const char *text)
{
    SetNavBarMidBtn(navBarWidget, button, NULL, NULL);
    switch (button) {
    case NVS_BAR_MID_WORD_SELECT:
        lv_label_set_text(lv_obj_get_child(navBarWidget->midBtn, 0), text);
        break;
    case NVS_BAR_MID_LABEL:
        lv_label_set_text(navBarWidget->midBtn, text);
        lv_obj_clear_flag(navBarWidget->midBtn, LV_OBJ_FLAG_HIDDEN);
        // GuiNvsBarSetMidCb(NVS_BAR_MID_LABEL, NULL, NULL);
        break;
    default:
        return;
    }
}

void SetRightBtnLabel(NavBarWidget_t *navBarWidget, NVS_RIGHT_BUTTON_ENUM button, const char *text)
{
    SetNavBarRightBtn(navBarWidget, button, NULL, NULL);
    switch (button) {
    case NVS_BAR_WORD_SELECT:
        lv_label_set_text(lv_obj_get_child(navBarWidget->rightBtn, 0), text);
        break;
    case NVS_BAR_WORD_RESET:
        lv_label_set_text(lv_obj_get_child(navBarWidget->rightBtn, 0), text);
        break;
    default:
        return;
    }
}

void SetRightBtnCb(NavBarWidget_t *navBarWidget, lv_event_cb_t eventCb, void *param)
{
    lv_obj_add_event_cb(navBarWidget->rightBtn, eventCb, LV_EVENT_CLICKED, param);
}

void SetNavBarRightBtn(NavBarWidget_t *navBarWidget, NVS_RIGHT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param)
{
    if (navBarWidget->rightBtn != NULL && lv_obj_is_valid(navBarWidget->rightBtn)) {
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
    default:
        return;
    }
    lv_obj_clear_flag(navBarWidget->rightBtn, LV_OBJ_FLAG_HIDDEN);
    if (rightButtonCb != NULL) {
        lv_obj_add_event_cb(navBarWidget->rightBtn, rightButtonCb, LV_EVENT_CLICKED, param);
    }
}