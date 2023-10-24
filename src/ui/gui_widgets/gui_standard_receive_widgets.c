#include "gui_standard_receive_widgets.h"
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
#include "inttypes.h"
#include "gui_home_widgets.h"
#include "gui_fullscreen_mode.h"
#include "keystore.h"
#include "gui_page.h"
#include "gui.h"
#include "gui_tutorial_widgets.h"
#include "account_manager.h"

#define GENERAL_ADDRESS_INDEX_MAX 999999999
#define LEDGER_LIVE_ADDRESS_INDEX_MAX 9

typedef enum {
    RECEIVE_TILE_QRCODE = 0,
    RECEIVE_TILE_SWITCH_ACCOUNT,

    RECEIVE_TILE_BUTT,
} StandardReceiveTile;

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
} PathWidgetsItem_t;

typedef struct {
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *tileQrCode;
    lv_obj_t *tileSwitchAccount;
    lv_obj_t *tileChangePath;
    lv_obj_t *attentionCont;
    lv_obj_t *qrCodeCont;
    lv_obj_t *qrCode;
    lv_obj_t *addressLabel;
    lv_obj_t *addressCountLabel;
    lv_obj_t *moreCont;
    lv_obj_t *addressButton;
    lv_obj_t *leftBtnImg;
    lv_obj_t *rightBtnImg;
    lv_obj_t *inputAddressLabel;
    lv_obj_t *overflowLabel;
    SwitchAddressWidgetsItem_t switchAddressWidgets[5];
} StandardReceiveWidgets_t;

typedef struct {
    uint32_t index;
    char address[128];
    char path[32];
} AddressDataItem_t;

typedef struct {
    char title[32];
    char subTitle[32];
    char path[32];
} PathItem_t;

static void GuiCreateMoreWidgets(lv_obj_t *parent);
static void GuiStandardReceiveGotoTile(StandardReceiveTile tile);
static void GuiCreateQrCodeWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressButtons(lv_obj_t *parent);

static void RefreshQrCode(void);
static void RefreshSwitchAccount(void);
static int GetMaxAddressIndex(void);
static void GetAttentionText(char* text);

static void CloseAttentionHandler(lv_event_t *e);
static void MoreHandler(lv_event_t *e);
static void TutorialHandler(lv_event_t *e);
static void LeftBtnHandler(lv_event_t *e);
static void RightBtnHandler(lv_event_t *e);
static bool IsAccountSwitchable();
static bool HasMoreBtn();
static void SwitchAddressHandler(lv_event_t *e);
static void OpenSwitchAddressHandler(lv_event_t *e);
static void AddressLongModeCut(char *out, const char *address);
static void SetCurrentSelectIndex(uint32_t selectIndex);
static uint32_t GetCurrentSelectIndex();

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item);

static StandardReceiveWidgets_t g_standardReceiveWidgets;
static StandardReceiveTile g_StandardReceiveTileNow;
static HOME_WALLET_CARD_ENUM g_chainCard;

// to do: stored.
static uint32_t g_showIndex;
static uint32_t g_selectIndex[3] = {0};
static uint32_t g_suiSelectIndex[3] = {0};
static uint32_t g_aptosSelectIndex[3] = {0};
static PageWidget_t *g_pageWidget;

void GuiStandardReceiveInit(uint8_t chain)
{
    g_chainCard = chain;
    g_pageWidget = CreatePageWidget();
    g_standardReceiveWidgets.cont = g_pageWidget->contentZone;
    g_standardReceiveWidgets.tileView = lv_tileview_create(g_standardReceiveWidgets.cont);
    lv_obj_set_style_bg_opa(g_standardReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_standardReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_standardReceiveWidgets.tileQrCode = lv_tileview_add_tile(g_standardReceiveWidgets.tileView, RECEIVE_TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_standardReceiveWidgets.tileQrCode);
    if (IsAccountSwitchable()) {
        g_standardReceiveWidgets.tileSwitchAccount = lv_tileview_add_tile(g_standardReceiveWidgets.tileView, RECEIVE_TILE_SWITCH_ACCOUNT, 0, LV_DIR_HOR);
        GuiCreateSwitchAddressWidget(g_standardReceiveWidgets.tileSwitchAccount);
        GuiCreateSwitchAddressButtons(g_standardReceiveWidgets.tileSwitchAccount);
    }
    lv_obj_clear_flag(g_standardReceiveWidgets.tileView, LV_OBJ_FLAG_SCROLLABLE);
}

void GuiStandardReceiveDeInit(void)
{
    GUI_DEL_OBJ(g_standardReceiveWidgets.moreCont)
    GUI_DEL_OBJ(g_standardReceiveWidgets.attentionCont)
    GUI_DEL_OBJ(g_standardReceiveWidgets.cont)

    CLEAR_OBJECT(g_standardReceiveWidgets);
    g_StandardReceiveTileNow = 0;
    GuiFullscreenModeCleanUp();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiStandardReceiveRefresh(void)
{
    char title[30];
    switch (g_StandardReceiveTileNow) {
    case RECEIVE_TILE_QRCODE:
        snprintf(title, sizeof(title), _("receive_coin_fmt"), GetCoinCardByIndex(g_chainCard)->coin);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainCard, title);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, HasMoreBtn() ? NVS_BAR_MORE_INFO : NVS_RIGHT_BUTTON_BUTT, MoreHandler, NULL);
        RefreshQrCode();
        break;
    case RECEIVE_TILE_SWITCH_ACCOUNT:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("switch_account"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        g_showIndex = GetCurrentSelectIndex() / 5 * 5;
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        } else if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        } else {
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        }
        break;
    default:
        break;
    }
}

void GuiStandardReceivePrevTile(void)
{
    GuiStandardReceiveGotoTile(RECEIVE_TILE_QRCODE);
}

static void GuiCreateMoreWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *btn, *img, *label;

    g_standardReceiveWidgets.moreCont = GuiCreateHintBox(parent, 480, 132, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_standardReceiveWidgets.moreCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_standardReceiveWidgets.moreCont);
    cont = g_standardReceiveWidgets.moreCont;

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
    label = GuiCreateLabelWithFont(btn, _("Tutorial"), &openSans_24);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);
}

static void GuiStandardReceiveGotoTile(StandardReceiveTile tile)
{
    g_StandardReceiveTileNow = tile;
    GuiStandardReceiveRefresh();
    lv_obj_set_tile_id(g_standardReceiveWidgets.tileView, g_StandardReceiveTileNow, 0, LV_ANIM_OFF);
}

lv_obj_t* CreateStandardReceiveQRCode(lv_obj_t* parent, uint16_t w, uint16_t h)
{
    lv_obj_t* qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    lv_qrcode_update(qrcode, "", 0);
    return qrcode;
}

static uint16_t GetAddrYExtend(void)
{
    if (g_chainCard == HOME_WALLET_CARD_SUI || g_chainCard == HOME_WALLET_CARD_APT) {
        return 30;
    }
    return 0;
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *tempObj;
    uint16_t yOffset = 0;
    uint16_t addrYExtend = GetAddrYExtend();

    g_standardReceiveWidgets.qrCodeCont = GuiCreateContainerWithParent(parent, 408, 524 + addrYExtend);
    lv_obj_align(g_standardReceiveWidgets.qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_standardReceiveWidgets.qrCodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_standardReceiveWidgets.qrCodeCont, 24, LV_PART_MAIN);

    yOffset += 36;
    g_standardReceiveWidgets.qrCode = CreateStandardReceiveQRCode(g_standardReceiveWidgets.qrCodeCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(CreateStandardReceiveQRCode, 420, 420);

    lv_obj_align(g_standardReceiveWidgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 16;
    g_standardReceiveWidgets.addressLabel = GuiCreateNoticeLabel(g_standardReceiveWidgets.qrCodeCont, "");
    lv_obj_set_width(g_standardReceiveWidgets.addressLabel, 336);
    lv_obj_align(g_standardReceiveWidgets.addressLabel, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 60;

    yOffset += 16;
    g_standardReceiveWidgets.addressCountLabel = GuiCreateIllustrateLabel(g_standardReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_standardReceiveWidgets.addressCountLabel, LV_ALIGN_TOP_LEFT, 36, yOffset + addrYExtend);

    g_standardReceiveWidgets.addressButton = lv_btn_create(g_standardReceiveWidgets.qrCodeCont);
    lv_obj_set_size(g_standardReceiveWidgets.addressButton, 336, 36);
    lv_obj_align(g_standardReceiveWidgets.addressButton, LV_ALIGN_TOP_MID, 0, 464 + addrYExtend);
    lv_obj_set_style_bg_opa(g_standardReceiveWidgets.addressButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_standardReceiveWidgets.addressButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_standardReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_standardReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    if (IsAccountSwitchable()) {
        lv_obj_add_event_cb(g_standardReceiveWidgets.addressButton, OpenSwitchAddressHandler, LV_EVENT_CLICKED, NULL);
        tempObj = GuiCreateImg(g_standardReceiveWidgets.addressButton, &imgArrowRight);
        lv_obj_set_style_img_opa(tempObj, LV_OPA_56, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_CENTER, 150, 0);
    }

    const char* coin = GetCoinCardByIndex(g_chainCard)->coin;
    if (!GetFirstReceive(coin)) {
        g_standardReceiveWidgets.attentionCont = GuiCreateHintBox(parent, 480, 386, false);
        tempObj = GuiCreateImg(g_standardReceiveWidgets.attentionCont, &imgInformation);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 462);
        tempObj = GuiCreateLittleTitleLabel(g_standardReceiveWidgets.attentionCont, _("Attention"));
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 558);
        char attentionText[150];
        GetAttentionText(attentionText);
        tempObj = GuiCreateLabelWithFont(g_standardReceiveWidgets.attentionCont, attentionText, &openSans_20);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 610);
        tempObj = GuiCreateBtn(g_standardReceiveWidgets.attentionCont, _("got_it"));
        lv_obj_set_size(tempObj, 122, 66);
        lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
        SetFirstReceive(coin, true);
    }
}

void GetAttentionText(char* text)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_TRX:
        strcpy(text, _("receive_trx_hint"));
        break;
    default:
        sprintf(text, _("receive_coin_hint_fmt"), GetCoinCardByIndex(g_chainCard)->coin);
    }
}

static void GuiCreateSwitchAddressWidget(lv_obj_t *parent)
{
    // Create the account list page.
    uint32_t index;
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 514);
    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    index = 0;
    for (uint32_t i = 0; i < 5; i++) {
        g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel = GuiCreateLabelWithFont(cont, "", &openSans_24);
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
        g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "");
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_standardReceiveWidgets.switchAddressWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, SwitchAddressHandler, LV_EVENT_CLICKED, NULL);

        g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg = GuiCreateImg(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg = GuiCreateImg(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

        index++;
    }
    RefreshSwitchAccount();
}

static void GuiCreateSwitchAddressButtons(lv_obj_t *parent)
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
    g_standardReceiveWidgets.leftBtnImg = img;

    btn = GuiCreateBtn(parent, "");
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    img = GuiCreateImg(btn, &imgArrowRight);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, RightBtnHandler, LV_EVENT_CLICKED, NULL);
    g_standardReceiveWidgets.rightBtnImg = img;
}

static void RefreshQrCode(void)
{
    AddressDataItem_t addressDataItem;

    ModelGetAddress(GetCurrentSelectIndex(), &addressDataItem);
    lv_qrcode_update(g_standardReceiveWidgets.qrCode, addressDataItem.address, strlen(addressDataItem.address));
    lv_obj_t *fullscreen_qrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullscreen_qrcode) {
        lv_qrcode_update(fullscreen_qrcode, addressDataItem.address, strlen(addressDataItem.address));
    }
    lv_label_set_text(g_standardReceiveWidgets.addressLabel, addressDataItem.address);
    lv_label_set_text_fmt(g_standardReceiveWidgets.addressCountLabel, "Account-%u", (addressDataItem.index + 1));
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "Account-%u", (addressDataItem.index + 1));
        AddressLongModeCut(string, addressDataItem.address);
        lv_label_set_text(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == GetCurrentSelectIndex()) {
            lv_obj_add_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == GetMaxAddressIndex()) {
            end = true;
        }
        index++;
    }
}

static int GetMaxAddressIndex(void)
{
    if (g_chainCard == HOME_WALLET_CARD_SUI || g_chainCard == HOME_WALLET_CARD_APT) {
        return 10;
    }
    return GENERAL_ADDRESS_INDEX_MAX;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(g_standardReceiveWidgets.attentionCont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void MoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_standardReceiveWidgets.moreCont == NULL) {
            GuiCreateMoreWidgets(g_standardReceiveWidgets.tileQrCode);
        } else {
            lv_obj_del(g_standardReceiveWidgets.moreCont);
            g_standardReceiveWidgets.moreCont = NULL;
        }
    }
}

static void TutorialHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_standardReceiveWidgets.moreCont);

        TUTORIAL_LIST_INDEX_ENUM index = TUTORIAL_ETH_RECEIVE;
        GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
    }
}

static void LeftBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex >= 5) {
            g_showIndex -= 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
    }
}

static void RightBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex < GetMaxAddressIndex() - 5) {
            g_showIndex += 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
    }
}

static bool IsAccountSwitchable()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_TRX:
    case HOME_WALLET_CARD_SUI:
    case HOME_WALLET_CARD_APT:
    case HOME_WALLET_CARD_XRP:
        return true;

    default:
        return false;
    }
}

static bool HasMoreBtn()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_TRX:
        return true;

    default:
        return false;
    }
}

static void SwitchAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkBox;

    if (code == LV_EVENT_CLICKED) {
        checkBox = lv_event_get_target(e);
        for (uint32_t i = 0; i < 5; i++) {
            if (checkBox == g_standardReceiveWidgets.switchAddressWidgets[i].checkBox) {
                lv_obj_add_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                SetCurrentSelectIndex(g_showIndex + i);
            } else {
                lv_obj_clear_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void OpenSwitchAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiStandardReceiveGotoTile(RECEIVE_TILE_SWITCH_ACCOUNT);
        RefreshSwitchAccount();
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

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item)
{
    char hdPath[128];
    sprintf(hdPath, "m/44'/195'/0'/0/%u", index);
    printf("hdPath=%s\r\n", hdPath);
    item->index = index;
    sprintf(item->address, "TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdH%010u", index);
    strcpy(item->path, hdPath);
}

#else

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub, hdPath[128];
    SimpleResponse_c_char *result;

    switch (g_chainCard) {
    case HOME_WALLET_CARD_TRX:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
        sprintf(hdPath, "m/44'/195'/0'/0/%u", index);
        result = tron_get_address(hdPath, xPub);
        break;
    case HOME_WALLET_CARD_SUI:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_SUI_0 + index);
        sprintf(hdPath, "m/44'/784'/%u'/0'/0'", index);
        result = sui_generate_address(xPub);
        break;
    case HOME_WALLET_CARD_APT:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_APT_0 + index);
        sprintf(hdPath, "m/44'/637'/%u'/0'/0'", index);
        result = aptos_generate_address(xPub);
        break;
    case HOME_WALLET_CARD_XRP:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);
        sprintf(hdPath, "m/44'/144'/0'/0/%u", index);
        result = xrp_get_address(hdPath, xPub, "m/44'/144'/0'/");
        break;

    default:
        if (IsCosmosChain(g_chainCard)) {
            char rootPath[128];
            const CosmosChain_t *chain = GuiGetCosmosChain(g_chainCard);
            sprintf(rootPath, "M/44'/%u'/0'", chain->coinType);
            sprintf(hdPath, "%s/0/%u", rootPath, index);
            xPub = GetCurrentAccountPublicKey(chain->xpubType);
            result = cosmos_get_address(hdPath, xPub, rootPath, (char*)chain->prefix);
        } else {
            printf("Standard Receive ModelGetAddress cannot match %d\r\n", index);
            return;
        }
    }
    ASSERT(xPub);

    if (result->error_code == 0) {
        item->index = index;
        strcpy(item->address, result->data);
        strcpy(item->path, hdPath);
    }
    free_simple_response_c_char(result);
}

#endif

void GuiResetCurrentStandardAddressIndex(uint8_t index)
{
    if(index > 2) {
        return;
    }
    g_selectIndex[index] = 0;
    g_suiSelectIndex[index] = 0;
    g_aptosSelectIndex[index] = 0;
}

void GuiResetAllStandardAddressIndex(void)
{
    memset(g_selectIndex, 0, sizeof(g_selectIndex));
    memset(g_suiSelectIndex, 0, sizeof(g_suiSelectIndex));
    memset(g_aptosSelectIndex, 0, sizeof(g_aptosSelectIndex));
}

static void SetCurrentSelectIndex(uint32_t selectIndex)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_SUI:
        g_suiSelectIndex[GetCurrentAccountIndex()] = selectIndex;
        break;
    case HOME_WALLET_CARD_APT:
        g_aptosSelectIndex[GetCurrentAccountIndex()] = selectIndex;
        break;
    default:
        g_selectIndex[GetCurrentAccountIndex()] = selectIndex;
        break;
    }
}

static uint32_t GetCurrentSelectIndex()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_SUI:
        return g_suiSelectIndex[GetCurrentAccountIndex()];
    case HOME_WALLET_CARD_APT:
        return g_aptosSelectIndex[GetCurrentAccountIndex()];
    default:
        return g_selectIndex[GetCurrentAccountIndex()];
    }
}
