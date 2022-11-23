/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_status_bar.c
 * Description:
 * author     : stone wang
 * data       : 2023-01-09 16:26
 **********************************************************************/
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
#ifndef COMPILE_SIMULATOR
#include "user_fatfs.h"
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

typedef struct NavBar {
    lv_obj_t *cont;
    // left
    lv_obj_t *returnBtn;
    lv_obj_t *closeBtn;
    lv_obj_t *manageBtn;

    // mid
    lv_obj_t *midLabel;
    lv_obj_t *midWordCntSelect;
    lv_obj_t *coinBtn;

    // right
    lv_obj_t *wordCntSelect;
    lv_obj_t *resetButton;
    lv_obj_t *question;
    lv_obj_t *moreInfo;
    lv_obj_t *skip;
    lv_obj_t *search;
    lv_obj_t *newSkip;
} NavBar_t;
static NavBar_t g_guiNavBar;

typedef struct {
    GuiChainCoinType index;
    const char *name;
    const lv_img_dsc_t *icon;
} CoinWalletInfo_t;

const static CoinWalletInfo_t g_coinWalletBtn[] = {
    {CHAIN_BTC, "Confirm Transaction", &coinBtc},
    {CHAIN_ETH, "Confirm Transaction", &coinEth},
    {CHAIN_BNB, "Confirm Transaction", &coinBnb},
    {CHAIN_SOL, "Confirm Transaction", &coinSol},
    {CHAIN_DOT, "Confirm Transaction", &coinDot},
    {CHAIN_XRP, "Confirm Transaction", &coinXrp},
    {CHAIN_LTC, "Confirm Transaction", &coinLtc},
    {CHAIN_DASH, "Confirm Transaction", &coinDash},
    {CHAIN_BCH, "Confirm Transaction", &coinBch},
    {CHAIN_TRX, "Confirm Transaction", &coinTrx},
};

const static CoinWalletInfo_t g_walletBtn[] = {
    {WALLET_LIST_KEYSTONE, "Connect Keystone Wallet", &walletKeystone},
    {WALLET_LIST_METAMASK, "Connect MetaMask", &walletMetamask},
    {WALLET_LIST_OKX, "Connect OKX Wallet", &walletOkx},
    {WALLET_LIST_BLUE, "Connect BlueWallet", &walletBluewallet},
    {WALLET_LIST_SUB, "Connect SubWallet", &walletSubwallet},
    {WALLET_LIST_SOLFARE, "Connect OKX Wallet", &walletSolflare},
    {WALLET_LIST_RABBY, "Connect Rabby", &walletRabby},
    {WALLET_LIST_SAFE, "Connect Safe", &walletSafe},
    {WALLET_LIST_BLOCK_WALLET, "Connect Block Wallet", &walletBlockWallet},
    {WALLET_LIST_ZAPPER, "Connect Zapper", &walletZapper},
    {WALLET_LIST_YEARN_FINANCE, "Connect Yearn Finance", &walletYearn},
    {WALLET_LIST_SUSHISWAP, "Connect SushiSwap", &walletSushi},
};

void GuiNvsBarSetLeftCb(NVS_LEFT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param)
{
    static lv_obj_t *leftButton = NULL;
    static lv_event_cb_t leftButtonCb = NULL;
    // if (leftButton != NULL && leftButtonCb != NULL) {
    if (leftButton != NULL) {
        lv_obj_remove_event_cb(leftButton, leftButtonCb);
        leftButtonCb = NULL;
        lv_obj_add_flag(leftButton, LV_OBJ_FLAG_HIDDEN);
        leftButton = NULL;
    }

    switch (button) {
    case NVS_BAR_RETURN:
        leftButton = g_guiNavBar.returnBtn;
        leftButtonCb = eventCb;
        break;
    case NVS_BAR_CLOSE:
        leftButton = g_guiNavBar.closeBtn;
        leftButtonCb = eventCb;
        break;
    case NVS_BAR_MANAGE:
        leftButton = g_guiNavBar.manageBtn;
        leftButtonCb = eventCb;
        break;
    default:
        return;
    }
    lv_obj_clear_flag(leftButton, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(leftButton, leftButtonCb, LV_EVENT_CLICKED, param);
}

void GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param)
{
    static lv_obj_t *rightButton = NULL;
    static lv_event_cb_t rightButtonCb = NULL;
    if (rightButton != NULL) {
        lv_obj_remove_event_cb(rightButton, rightButtonCb);
        rightButtonCb = NULL;
        lv_obj_add_flag(rightButton, LV_OBJ_FLAG_HIDDEN);
        rightButton = NULL;
    }

    switch (button) {
    case NVS_BAR_WORD_SELECT:
        rightButton = g_guiNavBar.wordCntSelect;
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_WORD_RESET:
        rightButton = g_guiNavBar.resetButton;
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_QUESTION_MARK:
        rightButton = g_guiNavBar.question;
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_MORE_INFO:
        rightButton = g_guiNavBar.moreInfo;
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_SKIP:
        rightButton = g_guiNavBar.skip;
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_SEARCH:
        rightButton = g_guiNavBar.search;
        rightButtonCb = eventCb;
        break;
    case NVS_BAR_NEW_SKIP:
        rightButton = g_guiNavBar.newSkip;
        rightButtonCb = eventCb;
        break;
    default:
        return;
    }
    lv_obj_clear_flag(rightButton, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(rightButton, rightButtonCb, LV_EVENT_CLICKED, param);
}

void GuiNvsBarSetRightBtnLabel(NVS_RIGHT_BUTTON_ENUM button, const char *text)
{
    switch (button) {
    case NVS_BAR_WORD_SELECT:
        lv_label_set_text(lv_obj_get_child(g_guiNavBar.wordCntSelect, 0), text);
        break;
    case NVS_BAR_WORD_RESET:
        lv_label_set_text(lv_obj_get_child(g_guiNavBar.resetButton, 0), text);
        break;
    default:
        return;
    }
}

void GuiNvsBarSetMidCb(NVS_MID_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param)
{
    static lv_obj_t *midButton = NULL;
    static lv_event_cb_t midButtonCb = NULL;
    // if (midButton != NULL && midButtonCb != NULL) {
    if (midButton != NULL) {
        lv_obj_remove_event_cb(midButton, midButtonCb);
        midButtonCb = NULL;
        lv_obj_add_flag(midButton, LV_OBJ_FLAG_HIDDEN);
        midButton = NULL;
    }

    switch (button) {
    case NVS_BAR_MID_WORD_SELECT:
        midButton = g_guiNavBar.midWordCntSelect;
        midButtonCb = eventCb;
        break;
    case NVS_BAR_MID_LABEL:
        midButton = g_guiNavBar.midLabel;
        midButtonCb = eventCb;
        break;
    case NVS_BAR_MID_COIN:
        midButton = g_guiNavBar.coinBtn;
        lv_obj_clear_flag(midButton, LV_OBJ_FLAG_HIDDEN);
        return;
    default:
        return;
    }
    lv_obj_clear_flag(midButton, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(midButton, midButtonCb, LV_EVENT_CLICKED, param);
}

void GuiNvsBarSetMidBtnLabel(NVS_MID_BUTTON_ENUM button, const char *text)
{
    switch (button) {
    case NVS_BAR_MID_WORD_SELECT:
        lv_label_set_text(lv_obj_get_child(g_guiNavBar.midWordCntSelect, 0), text);
        break;
    case NVS_BAR_MID_LABEL:
        lv_label_set_text(g_guiNavBar.midLabel, text);
        lv_obj_clear_flag(g_guiNavBar.midLabel, LV_OBJ_FLAG_HIDDEN);
        GuiNvsBarSetMidCb(NVS_BAR_MID_LABEL, NULL, NULL);
        break;
    default:
        return;
    }
}

void GuiNvsSetCoinWallet(GuiChainCoinType index, const char *name)
{
    g_guiNavBar.coinBtn = GuiUpdateStatusCoinButton(g_guiNavBar.coinBtn, (name != NULL) ? name : g_coinWalletBtn[index].name,
                          g_coinWalletBtn[index].icon);
    GuiNvsBarSetMidCb(NVS_BAR_MID_COIN, NULL, NULL);
}

void GuiNvsSetWallet(WALLET_LIST_INDEX_ENUM index, const char *name)
{
    if (name == NULL) {
        g_guiNavBar.coinBtn = GuiUpdateStatusCoinButton(g_guiNavBar.coinBtn, g_walletBtn[index].name,
                              g_walletBtn[index].icon);
    } else {
        g_guiNavBar.coinBtn = GuiUpdateStatusCoinButton(g_guiNavBar.coinBtn, name,
                              g_walletBtn[index].icon);
    }
    GuiNvsBarSetMidCb(NVS_BAR_MID_COIN, NULL, NULL);
}

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

    // 创建状态栏
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

    cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), GUI_NAV_BAR_HEIGHT);
    lv_obj_align_to(cont, g_guiStatusBar.cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT);
    lv_obj_set_style_radius(cont, 0, 0);
    g_guiNavBar.cont = cont;

    //returnbtn
    lv_obj_t *btn = GuiCreateBtn(cont, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    img = GuiCreateImg(btn, &imgArrowLeft);
    lv_obj_set_align(img, LV_ALIGN_CENTER);

    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.returnBtn = btn;

    //closebtn
    btn = GuiCreateBtn(cont, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    img = GuiCreateImg(btn, &imgClose);
    lv_obj_set_align(img, LV_ALIGN_CENTER);

    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.closeBtn = btn;

    //managebtn
    btn = GuiCreateBtn(cont, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 10, 0);

    img = GuiCreateImg(btn, &imgManage);
    lv_obj_set_align(img, LV_ALIGN_CENTER);

    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.manageBtn = btn;

    btn = GuiCreateBtnWithFont(cont, "24    " USR_SYMBOL_DOWN, &openSans_20);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 387, 0);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_size(btn, 69, 42);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    g_guiNavBar.wordCntSelect = btn;

    label = GuiCreateTextLabel(cont, "");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_guiNavBar.midLabel = label;

    btn = GuiCreateBtnWithFont(cont, "20    " USR_SYMBOL_DOWN, &openSans_20);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 268, 0);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_size(btn, 69, 42);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    g_guiNavBar.midWordCntSelect = btn;

    btn = GuiCreateStatusCoinButton(cont, _("Connect BlueWallet"), &walletBluewallet);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    g_guiNavBar.coinBtn = btn;
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);

    btn = GuiCreateBtnWithFont(cont, _("single_phrase_reset"), &openSansEnIllustrate);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(btn, 106, 42);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 350, 27);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.resetButton = btn;

    btn = GuiCreateBtn(cont, "");
    img = GuiCreateImg(btn, &imgQuestion);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_size(btn, 106, 42);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 380, 27);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.question = btn;

    btn = GuiCreateBtn(cont, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    img = GuiCreateImg(btn, &imgMore);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.moreInfo = btn;

    btn = GuiCreateBtn(cont, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    img = GuiCreateImg(btn, &imgSkip);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.skip = btn;

    btn = GuiCreateBtn(cont, "");
    lv_obj_set_size(btn, 64, 64);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -10, 0);
    img = GuiCreateImg(btn, &imgSearch);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_12, LV_STATE_PRESSED);
    g_guiNavBar.search = btn;


    btn = lv_label_create(cont);
    lv_label_set_text(btn, "");
    lv_obj_set_size(btn, 63, 42);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, GRAY_COLOR, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_10 + LV_OPA_2, LV_STATE_PRESSED | LV_PART_MAIN);

    lv_obj_t *textLabel = lv_label_create(btn);
    lv_label_set_text(textLabel, "Skip");
    lv_obj_set_style_text_font(textLabel, &openSans_20, LV_PART_MAIN);
    lv_obj_set_style_text_opa(textLabel, LV_OPA_90, LV_PART_MAIN);
    lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_bg_opa(textLabel, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_align(textLabel, LV_ALIGN_CENTER);
    lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 2);
    lv_obj_set_style_text_color(textLabel, WHITE_COLOR, LV_PART_MAIN);

    g_guiNavBar.newSkip = btn;

#ifdef COMPILE_SIMULATOR
    GuiStatusBarSetBattery(88, true);
#endif
}

void GuiNvsBarClear(void)
{
    GuiNvsBarSetLeftCb(NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "");
}

void GuiStatusBarSetSdCard(bool connected)
{
    char version[16] = {0};
    if (connected) {
        lv_obj_clear_flag(g_guiStatusBar.sdCardImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(g_guiStatusBar.sdCardImg, g_guiStatusBar.batteryImg, LV_ALIGN_OUT_LEFT_MID, -10, 0);
        uint8_t accountCnt = 0;
        GetExistAccountNum(&accountCnt);
        if (!GuiLockScreenIsTop() && FatfsGetSize("0:") && CheckOtaBinVersion(version) && accountCnt > 0) {
            GuiCreateSdCardUpdateHintbox(version);
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
