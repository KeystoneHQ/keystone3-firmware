/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Bitcoin receive address UI widgets.
 * Author: leon sun
 * Create: 2023-4-26
 ************************************************************************************************/

#include "gui_utxo_receive_widgets.h"
#include "gui_status_bar.h"
#include "gui_chain.h"
#include "gui_views.h"
#include "gui_hintbox.h"
#include "account_public_info.h"
#include "librust_c.h"
#include "assert.h"
#include "gui_keyboard.h"
#include "draw/lv_draw_mask.h"
#include "stdio.h"
#include "user_utils.h"
#include "gui_home_widgets.h"
#include "gui_button.h"
#include "gui_model.h"
#include "gui_tutorial_widgets.h"

#define ADDRESS_INDEX_MAX 999999999

typedef enum {
    UTXO_RECEIVE_TILE_QRCODE = 0,
    UTXO_RECEIVE_TILE_SWITCH_ACCOUNT,
    UTXO_RECEIVE_TILE_ADDRESS_SETTINGS,

    UTXO_RECEIVE_TILE_BUTT,
} UtxoReceiveTile;

typedef struct {
    lv_obj_t *addressCountLabel;
    lv_obj_t *addressLabel;
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} SwitchAddressWidgetsItem_t;

typedef struct {
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} addressSettingsWidgetsItem_t;

typedef struct {
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *tileQrCode;
    lv_obj_t *tileSwitchAccount;
    lv_obj_t *tileAddressSettings;
    lv_obj_t *attentionCont;
    lv_obj_t *qrCodeCont;
    lv_obj_t *qrCode;
    lv_obj_t *addressLabel;
    lv_obj_t *addressCountLabel;
    lv_obj_t *pathLabel;
    lv_obj_t *moreCont;
    lv_obj_t *addressButton;
    lv_obj_t *changeButton;
    lv_obj_t *changeImg;
    lv_obj_t *changeLabel;
    lv_obj_t *leftBtnImg;
    lv_obj_t *rightBtnImg;
    lv_obj_t *inputAddressCont;
    lv_obj_t *inputAddressLabel;
    lv_obj_t *overflowLabel;
    lv_obj_t *gotoAddressKeyboard;
    addressSettingsWidgetsItem_t addressSettingsWidgets[3];
    SwitchAddressWidgetsItem_t switchAddressWidgets[5];
} UtxoReceiveWidgets_t;

typedef struct {
    uint32_t index;
    char address[128];
    char path[32];
} AddressDataItem_t;

typedef struct {
    char title[32];
    char subTitle[32];
    char path[32];
} AddressSettingsItem_t;

typedef struct {
    HOME_WALLET_CARD_ENUM g_chainCard;
    char path[32];
} ChainPathItem_t;

typedef struct {
    GuiChainCoinType type;
    char tittle[32];
} TittleItem_t;

static void GuiCreateMoreWidgets(lv_obj_t *parent);
static void GuiBitcoinReceiveGotoTile(UtxoReceiveTile tile);
static void GuiCreateQrCodeWidget(lv_obj_t *parent);
static void GuiCreateSwitchAccountWidget(lv_obj_t *parent);
static void GuiCreateSwitchAccountButtons(lv_obj_t *parent);
static void GuiCreateAddressSettingsWidget(lv_obj_t *parent);
static void GuiCreateGotoAddressWidgets(lv_obj_t *parent);

static void RefreshQrCode(void);
static void RefreshSwitchAccount(void);

static void CloseAttentionHandler(lv_event_t *e);
static void MoreHandler(lv_event_t *e);
static void AddressSettingsHandler(lv_event_t *e);
static void TutorialHandler(lv_event_t *e);
static void LeftBtnHandler(lv_event_t *e);
static void RightBtnHandler(lv_event_t *e);
static void AddressSettingsCheckHandler(lv_event_t *e);
static void SwitchAddressHandler(lv_event_t *e);
static void SelectAddressHandler(lv_event_t *e);
static void ChangeAddressHandler(lv_event_t *e);
static void GotoAddressHandler(lv_event_t *e);
static void GotoAddressKeyboardHandler(lv_event_t *e);
static void CloseGotoAddressHandler(lv_event_t *e);

static void AddressLongModeCut(char *out, const char *address);
static void ModelGetUtxoAddress(uint32_t index, AddressDataItem_t *item);

static bool IsFirstAttention();
static void SetFirstAttention();
static void GetHint(char *hint);
static uint32_t GetCurrentSelectIndex();
static void SetCurrentSelectIndex(uint32_t selectIndex);
static void GetCurrentTittle(TittleItem_t *tittleItem);
static void SetKeyboardValid(bool);

static UtxoReceiveWidgets_t g_utxoReceiveWidgets;
static UtxoReceiveTile g_utxoReceiveTileNow;
static bool g_gotoAddressValid = false;
static const AddressSettingsItem_t g_addressSettings[] = {
    //{"Taproot",         "P2TR",             "m/86'/0'/0'"},
    {"Native SegWit", "P2WPKH", "m/84'/0'/0'"},
    {"Nested SegWit", "P2SH-P2WPKH", "m/49'/0'/0'"},
    {"Legacy", "P2PKH", "m/44'/0'/0'"},
    //{"Custom",          "Edit path",        "m/72'/0'/0'"}
};

static const ChainPathItem_t g_chainPathItems[] = {
    {HOME_WALLET_CARD_BTC, ""},
    {HOME_WALLET_CARD_LTC, "m/49'/2'/0'"},
    {HOME_WALLET_CARD_DASH, "m/44'/5'/0'"},
    {HOME_WALLET_CARD_BCH, "m/44'/145'/0'"}
};

static uint32_t g_showIndex;
static uint32_t g_selectIndex;

// to do: stored.
static uint32_t g_btcSelectIndex;
static uint32_t g_addressSettingsIndex = 0;
static uint32_t g_ltcSelectIndex;
static uint32_t g_dashSelectIndex;
static uint32_t g_bchSelectIndex;

static bool g_btcFirstAttention = false;
static bool g_ltcFirstAttention = false;
static bool g_dashFirstAttention = false;
static bool g_bchFirstAttention = false;

static HOME_WALLET_CARD_ENUM g_chainCard;

void GuiReceiveInit(uint8_t chain)
{
    g_chainCard = chain;
    g_selectIndex = GetCurrentSelectIndex();
    g_utxoReceiveWidgets.cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_align(g_utxoReceiveWidgets.cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);
    g_utxoReceiveWidgets.tileView = lv_tileview_create(g_utxoReceiveWidgets.cont);
    lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_utxoReceiveWidgets.tileQrCode = lv_tileview_add_tile(g_utxoReceiveWidgets.tileView, UTXO_RECEIVE_TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_utxoReceiveWidgets.tileQrCode);
    g_utxoReceiveWidgets.tileSwitchAccount = lv_tileview_add_tile(g_utxoReceiveWidgets.tileView, UTXO_RECEIVE_TILE_SWITCH_ACCOUNT, 0, LV_DIR_HOR);
    GuiCreateSwitchAccountWidget(g_utxoReceiveWidgets.tileSwitchAccount);
    GuiCreateSwitchAccountButtons(g_utxoReceiveWidgets.tileSwitchAccount);
    g_utxoReceiveWidgets.tileAddressSettings = lv_tileview_add_tile(g_utxoReceiveWidgets.tileView, UTXO_RECEIVE_TILE_ADDRESS_SETTINGS, 0, LV_DIR_HOR);
    GuiCreateAddressSettingsWidget(g_utxoReceiveWidgets.tileAddressSettings);
    lv_obj_clear_flag(g_utxoReceiveWidgets.tileView, LV_OBJ_FLAG_SCROLLABLE);
}

void GuiReceiveDeInit(void)
{
    if (g_utxoReceiveWidgets.moreCont != NULL) {
        lv_obj_del(g_utxoReceiveWidgets.moreCont);
        g_utxoReceiveWidgets.moreCont = NULL;
    }

    if (g_utxoReceiveWidgets.attentionCont != NULL) {
        lv_obj_del(g_utxoReceiveWidgets.attentionCont);
        g_utxoReceiveWidgets.attentionCont = NULL;
    }

    if (g_utxoReceiveWidgets.inputAddressCont != NULL) {
        lv_obj_del(g_utxoReceiveWidgets.inputAddressCont);
        g_utxoReceiveWidgets.inputAddressCont = NULL;
    }
    SetCurrentSelectIndex(g_selectIndex);
    lv_obj_del(g_utxoReceiveWidgets.cont);
    CLEAR_OBJECT(g_utxoReceiveWidgets);
    g_utxoReceiveTileNow = 0;
}

void GuiReceiveRefresh(void)
{
    printf("g_utxoReceiveTileNow=%d\r\n", g_utxoReceiveTileNow);
    switch (g_utxoReceiveTileNow) {
    case UTXO_RECEIVE_TILE_QRCODE:
        GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        TittleItem_t tittleItem;
        GetCurrentTittle(&tittleItem);
        GuiNvsSetCoinWallet(tittleItem.type, tittleItem.tittle);
        GuiNvsBarSetRightCb(NVS_BAR_MORE_INFO, MoreHandler, NULL);
        RefreshQrCode();
        if (g_selectIndex == ADDRESS_INDEX_MAX) {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.changeImg, LV_OPA_60, LV_PART_MAIN);
            lv_obj_set_style_text_opa(g_utxoReceiveWidgets.changeLabel, LV_OPA_60, LV_PART_MAIN);
        } else {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.changeImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_text_opa(g_utxoReceiveWidgets.changeLabel, LV_OPA_COVER, LV_PART_MAIN);
        }
        break;
    case UTXO_RECEIVE_TILE_SWITCH_ACCOUNT:
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "Switch Address");
        GuiNvsBarSetRightCb(NVS_BAR_SKIP, GotoAddressHandler, NULL);
        g_showIndex = g_selectIndex / 5 * 5;
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        } else if (g_showIndex >= ADDRESS_INDEX_MAX - 5) {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        } else {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        }
        break;
    case UTXO_RECEIVE_TILE_ADDRESS_SETTINGS:
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "Address Settings");
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    default:
        break;
    }
}

void GuiReceivePrevTile(void)
{
    GuiBitcoinReceiveGotoTile(UTXO_RECEIVE_TILE_QRCODE);
}

static void GetCurrentTittle(TittleItem_t *tittleItem)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        tittleItem->type = CHAIN_BTC;
        sprintf(tittleItem->tittle, "%s", "Receive BTC");
        break;
    case HOME_WALLET_CARD_LTC:
        tittleItem->type = CHAIN_LTC;
        sprintf(tittleItem->tittle, "%s", "Receive LTC");
        break;
    case HOME_WALLET_CARD_DASH:
        tittleItem->type = CHAIN_DASH;
        sprintf(tittleItem->tittle, "%s", "Receive DASH");
        break;
    case HOME_WALLET_CARD_BCH:
        tittleItem->type = CHAIN_BCH;
        sprintf(tittleItem->tittle, "%s", "Receive BCH");
        break;
    default:
        break;
    }
}

static void GuiCreateMoreWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *btn, *img, *label;
    int height = 228;
    if (g_chainCard != HOME_WALLET_CARD_BTC) {
        height = 132;
    }
    g_utxoReceiveWidgets.moreCont = GuiCreateHintBox(parent, 480, height, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_utxoReceiveWidgets.moreCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_utxoReceiveWidgets.moreCont);
    cont = g_utxoReceiveWidgets.moreCont;

    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        btn = lv_btn_create(cont);
        lv_obj_set_size(btn, 456, 84);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 24 + 572);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, AddressSettingsHandler, LV_EVENT_CLICKED, NULL);
        img = GuiCreateImg(btn, &imgAddressType);
        lv_obj_align(img, LV_ALIGN_CENTER, -186, 0);
        label = GuiCreateLabelWithFont(btn, "Address Settings", &openSans_24);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);
        break;
    default:
        break;
    }

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 456, 84);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 120 + 572);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, TutorialHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(btn, &imgTutorial);
    lv_obj_align(img, LV_ALIGN_CENTER, -186, 0);
    label = GuiCreateLabelWithFont(btn, "Tutorial", &openSans_24);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);
}

static void GuiBitcoinReceiveGotoTile(UtxoReceiveTile tile)
{
    g_utxoReceiveTileNow = tile;
    GuiReceiveRefresh();
    lv_obj_set_tile_id(g_utxoReceiveWidgets.tileView, g_utxoReceiveTileNow, 0, LV_ANIM_OFF);
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *tempObj;
    uint16_t yOffset = 0;

    g_utxoReceiveWidgets.qrCodeCont = GuiCreateContainerWithParent(parent, 408, 552);
    lv_obj_align(g_utxoReceiveWidgets.qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_utxoReceiveWidgets.qrCodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_utxoReceiveWidgets.qrCodeCont, 24, LV_PART_MAIN);

    yOffset += 36;
    g_utxoReceiveWidgets.qrCode = lv_qrcode_create(g_utxoReceiveWidgets.qrCodeCont, 336, BLACK_COLOR, WHITE_COLOR);
    lv_obj_align(g_utxoReceiveWidgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 16;
    g_utxoReceiveWidgets.addressLabel = GuiCreateNoticeLabel(g_utxoReceiveWidgets.qrCodeCont, "");
    lv_obj_set_width(g_utxoReceiveWidgets.addressLabel, 336);
    lv_obj_align(g_utxoReceiveWidgets.addressLabel, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 60;

    yOffset += 16;
    g_utxoReceiveWidgets.addressCountLabel = GuiCreateIllustrateLabel(g_utxoReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_utxoReceiveWidgets.addressCountLabel, LV_ALIGN_TOP_LEFT, 36, yOffset);
    yOffset += 30;

    yOffset += 4;
    g_utxoReceiveWidgets.pathLabel = GuiCreateNoticeLabel(g_utxoReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_utxoReceiveWidgets.pathLabel, LV_ALIGN_TOP_LEFT, 36, yOffset);

    g_utxoReceiveWidgets.addressButton = lv_btn_create(g_utxoReceiveWidgets.qrCodeCont);
    lv_obj_set_size(g_utxoReceiveWidgets.addressButton, 336, 64);
    lv_obj_align(g_utxoReceiveWidgets.addressButton, LV_ALIGN_TOP_MID, 0, 464);
    lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.addressButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_utxoReceiveWidgets.addressButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_utxoReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_utxoReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(g_utxoReceiveWidgets.addressButton, SelectAddressHandler, LV_EVENT_CLICKED, NULL);
    tempObj = GuiCreateImg(g_utxoReceiveWidgets.addressButton, &imgArrowRight);
    lv_obj_set_style_img_opa(tempObj, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(tempObj, LV_ALIGN_CENTER, 150, 0);

    g_utxoReceiveWidgets.changeButton = lv_btn_create(parent);
    lv_obj_set_size(g_utxoReceiveWidgets.changeButton, 305, 66);
    lv_obj_align(g_utxoReceiveWidgets.changeButton, LV_ALIGN_BOTTOM_LEFT, 88, -24);
    lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.changeButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_utxoReceiveWidgets.changeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_utxoReceiveWidgets.changeButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_utxoReceiveWidgets.changeButton, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(g_utxoReceiveWidgets.changeButton, ChangeAddressHandler, LV_EVENT_CLICKED, NULL);

    g_utxoReceiveWidgets.changeImg = GuiCreateImg(g_utxoReceiveWidgets.changeButton, &imgChange);
    g_utxoReceiveWidgets.changeLabel = GuiCreateTextLabel(parent, "Generate New Address");

    GuiButton_t table[2];
    table[0].obj = g_utxoReceiveWidgets.changeImg;
    table[0].position.x = -lv_obj_get_self_width(g_utxoReceiveWidgets.changeLabel) / 2 - 5;
    table[0].position.y = 0;
    table[0].align = LV_ALIGN_CENTER;

    table[1].obj = g_utxoReceiveWidgets.changeLabel;
    table[1].position.x = lv_obj_get_self_width(g_utxoReceiveWidgets.changeImg) / 2 + 5;
    table[1].position.y = 0;
    table[1].align = LV_ALIGN_CENTER;
    lv_obj_t *button = GuiCreateButton(parent, 350, 66, table, 2, NULL, NULL);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -24);

    if (IsFirstAttention()) {
        SetFirstAttention();
        g_utxoReceiveWidgets.attentionCont = GuiCreateHintBox(parent, 480, 386, false);
        tempObj = GuiCreateImg(g_utxoReceiveWidgets.attentionCont, &imgInformation);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 462);
        tempObj = GuiCreateLittleTitleLabel(g_utxoReceiveWidgets.attentionCont, "Attention");
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 558);
        char hint[256];
        GetHint(hint);
        tempObj = GuiCreateLabelWithFont(g_utxoReceiveWidgets.attentionCont, hint, &openSans_20);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 610);
        tempObj = GuiCreateBtn(g_utxoReceiveWidgets.attentionCont, "Got It");
        lv_obj_set_size(tempObj, 122, 66);
        lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
    }
}

static void GetHint(char *hint)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        strcpy(hint, "This address is exclusively for BTC transactions only. Sending other types of digital assets to this address will result in their loss.");
        break;
    case HOME_WALLET_CARD_LTC:
        strcpy(hint, "This address is only for LTC, other digital assets sent to this address will be lost.");
        break;
    case HOME_WALLET_CARD_DASH:
        strcpy(hint, "This address is only for DASH, other digital assets sent to this address will be lost.");
        break;
    case HOME_WALLET_CARD_BCH:
        strcpy(hint, "This address is only for BCH, other digital assets sent to this address will be lost.");
        break;
    default:
        break;
    }
}

static bool IsFirstAttention()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        return !g_btcFirstAttention;
    case HOME_WALLET_CARD_LTC:
        return !g_ltcFirstAttention;
    case HOME_WALLET_CARD_DASH:
        return !g_dashFirstAttention;
    case HOME_WALLET_CARD_BCH:
        return !g_bchFirstAttention;
    default:
        break;
    }

    return false;
}

static void SetFirstAttention()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        g_btcFirstAttention = true;
        break;
    case HOME_WALLET_CARD_LTC:
        g_ltcFirstAttention = true;
        break;
    case HOME_WALLET_CARD_DASH:
        g_dashFirstAttention = true;
        break;
    case HOME_WALLET_CARD_BCH:
        g_bchFirstAttention = true;
        break;
    default:
        break;
    }
}

static uint32_t GetCurrentSelectIndex()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        return g_btcSelectIndex;
    case HOME_WALLET_CARD_LTC:
        return g_ltcSelectIndex;
    case HOME_WALLET_CARD_DASH:
        return g_dashSelectIndex;
    case HOME_WALLET_CARD_BCH:
        return g_bchSelectIndex;
    default:
        break;
    }
    return g_btcSelectIndex;
}

static void SetCurrentSelectIndex(uint32_t selectIndex)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        g_btcSelectIndex = selectIndex;
        break;
    case HOME_WALLET_CARD_LTC:
        g_ltcSelectIndex = selectIndex;
        break;
    case HOME_WALLET_CARD_DASH:
        g_dashSelectIndex = selectIndex;
        break;
    case HOME_WALLET_CARD_BCH:
        g_bchSelectIndex = selectIndex;
        break;
    default:
        break;
    }
}

static void GuiCreateSwitchAccountWidget(lv_obj_t *parent)
{
    // Create the account list page.
    AddressDataItem_t addressDataItem;
    uint32_t index;
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 514);
    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    index = 0;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetUtxoAddress(index, &addressDataItem);
        g_utxoReceiveWidgets.switchAddressWidgets[i].addressCountLabel = GuiCreateLabelWithFont(cont, "", &openSans_24);
        lv_obj_align(g_utxoReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
        g_utxoReceiveWidgets.switchAddressWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "");
        lv_obj_align(g_utxoReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, SwitchAddressHandler, LV_EVENT_CLICKED, NULL);

        g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg = GuiCreateImg(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg = GuiCreateImg(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

        index++;
    }
    RefreshSwitchAccount();
}

static void GuiCreateSwitchAccountButtons(lv_obj_t *parent)
{
    lv_obj_t *btn;
    lv_obj_t *img;

    btn = GuiCreateBtn(parent, "");
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    img = GuiCreateImg(btn, &imgArrowLeft);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    if (g_showIndex < 5) {
        lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    }
    lv_obj_add_event_cb(btn, LeftBtnHandler, LV_EVENT_CLICKED, NULL);
    g_utxoReceiveWidgets.leftBtnImg = img;

    btn = GuiCreateBtn(parent, "");
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    img = GuiCreateImg(btn, &imgArrowRight);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, RightBtnHandler, LV_EVENT_CLICKED, NULL);
    g_utxoReceiveWidgets.rightBtnImg = img;
}

static void GuiCreateAddressSettingsWidget(lv_obj_t *parent)
{
    lv_obj_t *cont, *line, *label;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};
    char string[64];

    cont = GuiCreateContainerWithParent(parent, 408, 514);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    for (uint32_t i = 0; i < 3; i++) {
        label = GuiCreateLabelWithFont(cont, g_addressSettings[i].title, &openSans_24);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
        sprintf(string, "%s (%s)", g_addressSettings[i].subTitle, g_addressSettings[i].path);
        label = GuiCreateNoticeLabel(cont, string);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        line = GuiCreateLine(cont, points, 2);
        lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * (i + 1));

        g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, AddressSettingsCheckHandler, LV_EVENT_CLICKED, NULL);

        g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg = GuiCreateImg(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg = GuiCreateImg(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_clear_flag(g_utxoReceiveWidgets.addressSettingsWidgets[g_addressSettingsIndex].checkedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[g_addressSettingsIndex].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
}

static void GuiCreateGotoAddressWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *label, *line, *closeBtn, *closeImg;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    g_gotoAddressValid = false;

    if (g_utxoReceiveWidgets.inputAddressCont == NULL) {
        g_utxoReceiveWidgets.inputAddressCont = GuiCreateHintBox(parent, 480, 530, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_utxoReceiveWidgets.inputAddressCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_utxoReceiveWidgets.inputAddressCont);
        cont = g_utxoReceiveWidgets.inputAddressCont;

        label = GuiCreateLabelWithFont(cont, "Go to", &openSans_20);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 30 + 270);
        lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
        label = GuiCreateLabelWithFont(cont, "Address-", &openSans_24);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 108 + 270);
        lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
        g_utxoReceiveWidgets.inputAddressLabel = GuiCreateLabelWithFont(cont, "", &openSans_24);
        lv_obj_align(g_utxoReceiveWidgets.inputAddressLabel, LV_ALIGN_TOP_LEFT, 38 + lv_obj_get_self_width(label), 108 + 270);
        label = GuiCreateLabelWithFont(cont, "Cannot exceed 999,999,999", &openSans_20);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 170 + 270);
        lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        g_utxoReceiveWidgets.overflowLabel = label;

        line = GuiCreateLine(cont, points, 2);
        lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 160 + 270);

        lv_obj_t *keyboard = GuiCreateNumKeyboard(cont, GotoAddressKeyboardHandler, NUM_KEYBOARD_NORMAL, NULL);
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_add_style(keyboard, &g_numBtnmStyle, LV_PART_ITEMS);
        lv_obj_add_style(keyboard, &g_enterPressBtnmStyle, LV_STATE_PRESSED | LV_PART_ITEMS);
        lv_btnmatrix_set_btn_ctrl(keyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
        g_utxoReceiveWidgets.gotoAddressKeyboard = keyboard;

        closeBtn = lv_btn_create(cont);
        lv_obj_set_size(closeBtn, 36, 36);
        lv_obj_align(closeBtn, LV_ALIGN_TOP_RIGHT, -36, 27 + 270);
        lv_obj_set_style_bg_opa(closeBtn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(closeBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(closeBtn, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(closeBtn, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(closeBtn, CloseGotoAddressHandler, LV_EVENT_CLICKED, NULL);
        closeImg = GuiCreateImg(closeBtn, &imgClose);
        lv_obj_align(closeImg, LV_ALIGN_CENTER, 0, 0);
    } else {
        lv_label_set_text(g_utxoReceiveWidgets.inputAddressLabel, "");
        lv_obj_clear_flag(g_utxoReceiveWidgets.inputAddressCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_utxoReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

static void RefreshQrCode(void)
{
    AddressDataItem_t addressDataItem;

    ModelGetUtxoAddress(g_selectIndex, &addressDataItem);
    lv_qrcode_update(g_utxoReceiveWidgets.qrCode, addressDataItem.address, strlen(addressDataItem.address));
    lv_label_set_text(g_utxoReceiveWidgets.addressLabel, addressDataItem.address);
    lv_label_set_text_fmt(g_utxoReceiveWidgets.addressCountLabel, "Address-%u", addressDataItem.index);
    lv_label_set_text(g_utxoReceiveWidgets.pathLabel, addressDataItem.path);
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetUtxoAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_utxoReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "Address-%u", addressDataItem.index);
        AddressLongModeCut(string, addressDataItem.address);
        lv_label_set_text(g_utxoReceiveWidgets.switchAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == g_selectIndex) {
            lv_obj_add_state(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == ADDRESS_INDEX_MAX) {
            end = true;
        }
        index++;
    }
}

static void CloseAttentionHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(g_utxoReceiveWidgets.attentionCont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void MoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_utxoReceiveWidgets.moreCont == NULL) {
            GuiCreateMoreWidgets(g_utxoReceiveWidgets.tileQrCode);
        } else {
            lv_obj_del(g_utxoReceiveWidgets.moreCont);
            g_utxoReceiveWidgets.moreCont = NULL;
        }
    }
}

static void AddressSettingsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_utxoReceiveWidgets.moreCont != NULL) {
            lv_obj_del(g_utxoReceiveWidgets.moreCont);
            g_utxoReceiveWidgets.moreCont = NULL;
        }
        GuiBitcoinReceiveGotoTile(UTXO_RECEIVE_TILE_ADDRESS_SETTINGS);
    }
}

static void TutorialHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_utxoReceiveWidgets.moreCont);
        if (g_chainCard == HOME_WALLET_CARD_BTC) {
            TUTORIAL_LIST_INDEX_ENUM tIndex = TUTORIAL_BTC_RECEIVE;
            GuiFrameOpenViewWithParam(&g_tutorialView, &tIndex, sizeof(tIndex));
        }
    }
}

static void LeftBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_utxoReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex >= 5) {
            g_showIndex -= 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
    }
}

static void RightBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_utxoReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex < ADDRESS_INDEX_MAX - 5) {
            g_showIndex += 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex >= ADDRESS_INDEX_MAX - 5) {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
    }
}

static void AddressSettingsCheckHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkBox;

    if (code == LV_EVENT_CLICKED) {
        checkBox = lv_event_get_target(e);
        for (uint32_t i = 0; i < 3; i++) {
            if (checkBox == g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox) {
                lv_obj_add_state(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_clear_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                if (g_addressSettingsIndex != i) {
                    g_addressSettingsIndex = i;
                    g_selectIndex = 0;
                }
            } else {
                lv_obj_clear_state(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void SwitchAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkBox;

    if (code == LV_EVENT_CLICKED) {
        checkBox = lv_event_get_target(e);
        for (uint32_t i = 0; i < 5; i++) {
            if (checkBox == g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox) {
                lv_obj_add_state(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                g_selectIndex = g_showIndex + i;
            } else {
                lv_obj_clear_state(g_utxoReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_add_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_utxoReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void ChangeAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (g_selectIndex < ADDRESS_INDEX_MAX) {
            g_selectIndex++;
        }
        RefreshQrCode();
        if (g_selectIndex == ADDRESS_INDEX_MAX) {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.changeImg, LV_OPA_60, LV_PART_MAIN);
            lv_obj_set_style_text_opa(g_utxoReceiveWidgets.changeLabel, LV_OPA_60, LV_PART_MAIN);
        } else {
            lv_obj_set_style_img_opa(g_utxoReceiveWidgets.changeImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_text_opa(g_utxoReceiveWidgets.changeLabel, LV_OPA_COVER, LV_PART_MAIN);
        }
    }
}

static void SelectAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiBitcoinReceiveGotoTile(UTXO_RECEIVE_TILE_SWITCH_ACCOUNT);
        RefreshSwitchAccount();
    }
}

static void GotoAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiCreateGotoAddressWidgets(g_utxoReceiveWidgets.tileSwitchAccount);
    }
}

static void SetKeyboardValid(bool validation)
{
    if (validation) {
        if (lv_btnmatrix_has_btn_ctrl(g_utxoReceiveWidgets.gotoAddressKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED)) {
            lv_btnmatrix_clear_btn_ctrl(g_utxoReceiveWidgets.gotoAddressKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
        }
        lv_btnmatrix_set_btn_ctrl(g_utxoReceiveWidgets.gotoAddressKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
    } else {
        if (lv_btnmatrix_has_btn_ctrl(g_utxoReceiveWidgets.gotoAddressKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED)) {
            lv_btnmatrix_clear_btn_ctrl(g_utxoReceiveWidgets.gotoAddressKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
        }
        lv_btnmatrix_set_btn_ctrl(g_utxoReceiveWidgets.gotoAddressKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
    }
}

static void GotoAddressKeyboardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    lv_obj_draw_part_dsc_t *dsc;
    const char *txt;
    char input[16];
    uint32_t len;
    uint64_t longInt;

    if (code == LV_EVENT_CLICKED) {
        txt = lv_btnmatrix_get_btn_text(obj, id);
        strcpy(input, lv_label_get_text(g_utxoReceiveWidgets.inputAddressLabel));
        if (strcmp(txt, LV_SYMBOL_OK) == 0) {
            if (g_gotoAddressValid) {
                sscanf(input, "%u", &g_selectIndex);
                g_showIndex = g_selectIndex / 5 * 5;
                RefreshSwitchAccount();
                GuiReceiveRefresh();
                lv_obj_add_flag(g_utxoReceiveWidgets.inputAddressCont, LV_OBJ_FLAG_HIDDEN);
                g_gotoAddressValid = false;
            }
        } else if (strcmp(txt, "-") == 0) {
            len = strlen(input);
            if (len >= 1) {
                input[len - 1] = '\0';
                lv_label_set_text(g_utxoReceiveWidgets.inputAddressLabel, input);
                lv_obj_add_flag(g_utxoReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
                if (strlen(input) >= 1) {
                    g_gotoAddressValid = true;
                } else {
                    g_gotoAddressValid = false;
                }
            }
        } else if (strlen(input) < 15) {
            strcat(input, txt);
            longInt = strtol(input, NULL, 10);
            if (longInt >= ADDRESS_INDEX_MAX) {
                input[9] = '\0';
                lv_obj_clear_flag(g_utxoReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_utxoReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            }
            if (longInt > 0) {
                if (input[0] == '0') {
                    lv_label_set_text(g_utxoReceiveWidgets.inputAddressLabel, input + 1);
                } else {
                    lv_label_set_text(g_utxoReceiveWidgets.inputAddressLabel, input);
                }
            } else {
                lv_label_set_text(g_utxoReceiveWidgets.inputAddressLabel, "0");
            }
            g_gotoAddressValid = true;
        } else {
            g_gotoAddressValid = false;
            printf("input to long\r\n");
        }
        SetKeyboardValid(g_gotoAddressValid);
    } else if (code == LV_EVENT_DRAW_PART_BEGIN) {
        dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            /*Change the draw descriptor of the 12th button*/
            if (dsc->id == 9) {
                dsc->label_dsc->opa = LV_OPA_TRANSP;
            } else if (dsc->id == 11) {
                dsc->rect_dsc->bg_color = ORANGE_COLOR;
                dsc->label_dsc->opa = LV_OPA_TRANSP;
            } else {
                dsc->rect_dsc->bg_color = DARK_GRAY_COLOR;
            }
        }
    } else if (code == LV_EVENT_DRAW_PART_END) {
        dsc = lv_event_get_draw_part_dsc(e);
        /*When the button matrix draws the buttons...*/
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            /*Add custom content to the 4th button when the button itself was drawn*/
            if (dsc->id == 9 || dsc->id == 11) {
                lv_img_header_t header;
                lv_draw_img_dsc_t img_draw_dsc;
                lv_area_t a;
                const lv_img_dsc_t *imgDsc;
                lv_res_t res;
                imgDsc = dsc->id == 9 ? &imgBackspace : &imgCheck;
                res = lv_img_decoder_get_info(imgDsc, &header);
                if (res != LV_RES_OK)
                    return;
                a.x1 = dsc->draw_area->x1 + (lv_area_get_width(dsc->draw_area) - header.w) / 2;
                a.x2 = a.x1 + header.w - 1;
                a.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - header.h) / 2;
                a.y2 = a.y1 + header.h - 1;
                lv_draw_img_dsc_init(&img_draw_dsc);
                img_draw_dsc.recolor = lv_color_black();
                if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
                    img_draw_dsc.recolor_opa = LV_OPA_30;

                lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, imgDsc);
            }
        }
    }
}

static void CloseGotoAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(g_utxoReceiveWidgets.inputAddressCont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void AddressLongModeCut(char *out, const char *address)
{
    uint32_t len;

    len = strlen(address);
    if (len <= 24) {
        strcpy(out, address);
        return;
    }
    strncpy(out, address, 12);
    out[12] = 0;
    strcat(out, "...");
    strcat(out, address + len - 12);
}

#ifdef COMPILE_SIMULATOR

static void ModelGetUtxoAddress(uint32_t index, AddressDataItem_t *item)
{
    char hdPath[128];
    // sprintf(hdPath, "m/44'/0'/0'/0/%u", index);
    sprintf(hdPath, "%s/0/%u", g_addressSettings[g_addressSettingsIndex].path, index);
    item->index = index;
    sprintf(item->address, "tb1qkcp7vdhczgk5eh59d2l0dxvmpzhx%010u", index);
    strcpy(item->path, hdPath);
}

#else

static ChainType GetChanTypeByIndex(uint32_t index)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC: {
        if (index == 0) {
            return XPUB_TYPE_BTC_NATIVE_SEGWIT;
        } else if (index == 1) {
            return XPUB_TYPE_BTC;
        } else {
            return XPUB_TYPE_BTC_LEGACY;
        }
        break;
    }
    case HOME_WALLET_CARD_LTC:
        return XPUB_TYPE_LTC;
    case HOME_WALLET_CARD_DASH:
        return XPUB_TYPE_DASH;
    case HOME_WALLET_CARD_BCH:
        return XPUB_TYPE_BCH;
    default:
        break;
    }
    return XPUB_TYPE_BTC;
}

static void GetRootHdPath(char *hdPath)
{

    switch (g_chainCard) {
    case HOME_WALLET_CARD_BTC:
        sprintf(hdPath, "%s", g_addressSettings[g_addressSettingsIndex].path);
        break;
    case HOME_WALLET_CARD_LTC:
        sprintf(hdPath, "%s", g_chainPathItems[1].path);
        break;
    case HOME_WALLET_CARD_DASH:
        sprintf(hdPath, "%s", g_chainPathItems[2].path);
        break;
    case HOME_WALLET_CARD_BCH:
        sprintf(hdPath, "%s", g_chainPathItems[3].path);
        break;
    default:
        break;
    }
}

static void ModelGetUtxoAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub, rootPath[128], hdPath[128];
    ChainType chainType;
    chainType = GetChanTypeByIndex(g_addressSettingsIndex);
    xPub = GetCurrentAccountPublicKey(chainType);
    ASSERT(xPub);
    SimpleResponse_c_char *result;
    GetRootHdPath(rootPath);
    sprintf(hdPath, "%s/0/%u", rootPath, index);
    do {
        result = utxo_get_address(hdPath, xPub);
        CHECK_CHAIN_BREAK(result);
    } while (0);
    item->index = index;
    strcpy(item->address, result->data);
    strcpy(item->path, hdPath);
    free_simple_response_c_char(result);
}

#endif
