#include "gui_style.h"
#include "gui_obj.h"
#include "gui_button.h"
#ifndef BTC_ONLY
#include "gui_multi_accounts_receive_widgets.h"
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

#define GENERAL_ADDRESS_INDEX_MAX                           (999999999)
#define GENERAL_ACCOUNT_INDEX_MAX                           (999999999)
#define ACCOUNT_INDEX_MAX                                   (24)
#define INPUT_ADDRESS_MAX_LEN                               (16)

typedef enum {
    RECEIVE_TILE_QRCODE = 0,
    RECEIVE_TILE_SWITCH_ACCOUNT,
    RECEIVE_TILE_SWITCH_PATH_TYPE,

    RECEIVE_TILE_BUTT,
} MultiAccountsReceiveTile;

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
    lv_obj_t *tileSwitchAddress;
    lv_obj_t *tileSwitchPathType;
    lv_obj_t *attentionCont;
    lv_obj_t *qrCodeCont;
    lv_obj_t *qrCode;
    lv_obj_t *addressLabel;
    lv_obj_t *addressCountLabel;
    lv_obj_t *moreCont;
    lv_obj_t *addressButton;
    lv_obj_t *pathTypeButton;
    lv_obj_t *leftBtnImg;
    lv_obj_t *rightBtnImg;
    lv_obj_t *leftSwitchAccountBtnImg;
    lv_obj_t *rightSwitchAccountBtnImg;
    lv_obj_t *confirmAccountBtn;
    lv_obj_t *confirmIndexBtn;
    lv_obj_t *inputAccountLabel;
    lv_obj_t *overflowLabel;
    lv_obj_t *inputAccountCont;
    lv_obj_t *inputAccountKeyboard;
    lv_obj_t *addressDetailCont;
    PageWidget_t *switchAccountCont;
    lv_obj_t *questionImg;
    SwitchAddressWidgetsItem_t switchAddressWidgets[5];
    SwitchAddressWidgetsItem_t switchAccountWidgets[5];
} MultiAccountsReceiveWidgets_t;

typedef struct {
    uint32_t index;
    char address[ADDRESS_MAX_LEN];
    char path[PATH_ITEM_MAX_LEN];
} AddressDataItem_t;

typedef struct {
    char title[PATH_ITEM_MAX_LEN];
    char subTitle[PATH_ITEM_MAX_LEN];
    char path[PATH_ITEM_MAX_LEN];
} PathItem_t;

static void GuiCreateMoreWidgets(lv_obj_t *parent);
static void GuiMultiAccountsReceiveGotoTile(MultiAccountsReceiveTile tile);
static void GuiCreateQrCodeWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressButtons(lv_obj_t *parent);
static void GuiCreateSwitchAccountButtons(lv_obj_t *parent);
static void GuiCreateGotoAddressWidgets(lv_obj_t *parent);
static void GuiCreateAddressDetailWidgets(lv_obj_t *parent);
static void GuiCreateSwitchAccountWidget();

static void RefreshQrCode(void);
static void RefreshSwitchAddress(void);
static int GetMaxAddressIndex(void);
static void GetAttentionText(char *text);
static void SetKeyboardValid(bool);

static void CloseAttentionHandler(lv_event_t *e);
static void MoreHandler(lv_event_t *e);
static void TutorialHandler(lv_event_t *e);
static void JumpToAccountHandler(lv_event_t *e);
static void InputAddressIndexKeyboardHandler(lv_event_t *e);
static void LeftBtnHandler(lv_event_t *e);
static void RightBtnHandler(lv_event_t *e);
static void LeftSwitchAccountBtnHandler(lv_event_t *e);
static void RightSwitchAccountBtnHandler(lv_event_t *e);
static int GetMaxAccountIndex(void);
static bool IsAddressSwitchable();
static bool IsPathTypeSwitchable();
static bool HasMoreBtn();
static void SwitchAddressHandler(lv_event_t *e);
static void SwitchAccountHandler(lv_event_t *e);

static void OpenSwitchAccountHandler(lv_event_t *e);
static void CloseSwitchAccountHandler(lv_event_t *e);
static void RefreshSwitchAccount(void);

static void OpenSwitchAddressHandler(lv_event_t *e);
static void CloseSwitchAddressHandler(lv_event_t *e);

static void OpenChangePathTypeHandler(lv_event_t *e);

static void ShowAddressDetailHandler(lv_event_t *e);
static void UpdateConfirmIndexBtn(void);
static void UpdateConfirmAccountBtn(void);
static void ChangePathTypeReturnHandler(lv_event_t *e);
static void PathTypeChangedCb(lv_event_t *e);
static void SetCurrentSelectIndex(uint32_t index);

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item, uint8_t type);

static MultiAccountsReceiveWidgets_t g_multiAccountsReceiveWidgets;
static MultiAccountsReceiveTile g_multiAccountsReceiveTileNow;
static HOME_WALLET_CARD_ENUM g_chainCard;

// to do: stored.
static uint32_t g_showIndex;
static uint32_t g_showAccountIndex;
static uint32_t g_selectedIndex[3] = {0};
static uint32_t g_selectedAccount[3] = {0};
static PageWidget_t *g_pageWidget;
static bool g_inputAccountValid = true;
static uint32_t g_tmpIndex = 0;
static uint32_t g_tmpAccount = 0;

void GuiMultiAccountsReceiveInit(uint8_t chain)
{
    g_chainCard = chain;
    g_pageWidget = CreatePageWidget();
    g_multiAccountsReceiveWidgets.cont = g_pageWidget->contentZone;
    g_multiAccountsReceiveWidgets.tileView = lv_tileview_create(g_multiAccountsReceiveWidgets.cont);
    g_showAccountIndex = GetAccountIndex(GetCoinCardByIndex(g_chainCard)->coin);
    g_selectedAccount[GetCurrentAccountIndex()] = g_showAccountIndex;
    g_showIndex = GetAccountReceiveIndex(GetCoinCardByIndex(g_chainCard)->coin);
    g_selectedIndex[GetCurrentAccountIndex()] = g_showIndex;
    lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_multiAccountsReceiveWidgets.tileQrCode = lv_tileview_add_tile(g_multiAccountsReceiveWidgets.tileView, RECEIVE_TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_multiAccountsReceiveWidgets.tileQrCode);
    if (IsAddressSwitchable()) {
        g_multiAccountsReceiveWidgets.tileSwitchAddress = lv_tileview_add_tile(g_multiAccountsReceiveWidgets.tileView, RECEIVE_TILE_SWITCH_ACCOUNT, 0, LV_DIR_HOR);
        GuiCreateSwitchAddressWidget(g_multiAccountsReceiveWidgets.tileSwitchAddress);
        GuiCreateSwitchAddressButtons(g_multiAccountsReceiveWidgets.tileSwitchAddress);
    }
    if (IsPathTypeSwitchable()) {
        g_multiAccountsReceiveWidgets.tileSwitchPathType = lv_tileview_add_tile(g_multiAccountsReceiveWidgets.tileView, RECEIVE_TILE_SWITCH_PATH_TYPE, 0, LV_DIR_HOR);
    }
    lv_obj_clear_flag(g_multiAccountsReceiveWidgets.tileView, LV_OBJ_FLAG_SCROLLABLE);

    GuiMultiAccountsReceiveRefresh();
}

void GuiMultiAccountsReceiveDeInit(void)
{
    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.moreCont)
    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.attentionCont)
    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.cont)
    GUI_PAGE_DEL(g_multiAccountsReceiveWidgets.switchAccountCont)
    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.addressDetailCont)
    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.inputAccountCont)

    CLEAR_OBJECT(g_multiAccountsReceiveWidgets);
    GuiFullscreenModeCleanUp();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_multiAccountsReceiveTileNow = RECEIVE_TILE_QRCODE;
}

void GuiMultiAccountsReceiveRefresh(void)
{
    char title[30];
    switch (g_multiAccountsReceiveTileNow) {
    case RECEIVE_TILE_QRCODE:
        snprintf(title, sizeof(title), _("receive_coin_fmt"), GetCoinCardByIndex(g_chainCard)->coin);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainCard, title);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, HasMoreBtn() ? NVS_BAR_MORE_INFO : NVS_RIGHT_BUTTON_BUTT, MoreHandler, NULL);
        RefreshQrCode();
        break;
    case RECEIVE_TILE_SWITCH_ACCOUNT:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("switch_address"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SKIP, JumpToAccountHandler, NULL);
        g_tmpIndex = g_selectedIndex[GetCurrentAccountIndex()];
        g_showIndex = g_tmpIndex / 5 * 5;
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        } else if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        } else {
            lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        }
        UpdateConfirmIndexBtn();
        break;
    case RECEIVE_TILE_SWITCH_PATH_TYPE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ChangePathTypeReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("derivation_path_change"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, MoreHandler, NULL);
        GuiCreateSwitchPathTypeWidget(g_multiAccountsReceiveWidgets.tileSwitchPathType, g_chainCard, PathTypeChangedCb);
        printf("%s %d\n", __func__, __LINE__);
        break;
    default:
        break;
    }
}

static void PathTypeChangedCb(lv_event_t *e)
{
    SetCurrentSelectIndex(0);
}

static void ChangePathTypeReturnHandler(lv_event_t *e)
{
    GuiDestroySwitchPathTypeWidget();
    ReturnHandler(NULL);
}

void GuiMultiAccountsReceivePrevTile(void)
{
    GuiMultiAccountsReceiveGotoTile(RECEIVE_TILE_QRCODE);
}

static void GuiCreateMoreWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *btn, *img, *label;

    uint16_t height = 228;
    if (IsPathTypeSwitchable()) {
        height = 324;
    }
    g_multiAccountsReceiveWidgets.moreCont = GuiCreateHintBox(height);
    lv_obj_add_event_cb(lv_obj_get_child(g_multiAccountsReceiveWidgets.moreCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_multiAccountsReceiveWidgets.moreCont);
    cont = g_multiAccountsReceiveWidgets.moreCont;

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 456, 84);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 216 + 476);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, TutorialHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(btn, &imgTutorial);
    lv_obj_align(img, LV_ALIGN_CENTER, -186, 0);
    label = GuiCreateTextLabel(btn, _("Tutorial"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 456, 84);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 120 + 476);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, OpenSwitchAccountHandler, LV_EVENT_CLICKED, NULL);

    img = GuiCreateImg(btn, &imgAddressType);
    lv_obj_align(img, LV_ALIGN_CENTER, -186, 0);
    label = GuiCreateTextLabel(btn, _("switch_account"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);

    if (g_chainCard == HOME_WALLET_CARD_ADA) {
        btn = lv_btn_create(cont);
        lv_obj_set_size(btn, 456, 84);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 24 + 476);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, OpenChangePathTypeHandler, LV_EVENT_CLICKED, NULL);
        img = GuiCreateImg(btn, &imgPath);
        lv_obj_align(img, LV_ALIGN_CENTER, -186, 0);
        label = GuiCreateTextLabel(btn, _("derivation_path_change"));
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);
    }
}

static void GuiMultiAccountsReceiveGotoTile(MultiAccountsReceiveTile tile)
{
    g_multiAccountsReceiveTileNow = tile;
    GuiMultiAccountsReceiveRefresh();
    lv_obj_set_tile_id(g_multiAccountsReceiveWidgets.tileView, g_multiAccountsReceiveTileNow, 0, LV_ANIM_OFF);
}

lv_obj_t *CreateMultiAccountsReceiveQRCode(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    lv_qrcode_update(qrcode, "", 0);
    return qrcode;
}

static uint16_t GetAddrYExtend(void)
{
    if (g_chainCard == HOME_WALLET_CARD_SUI) {
        return 30;
    }
    if (g_chainCard == HOME_WALLET_CARD_MONERO) {
        return 60;
    }
    return 0;
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *tempObj;
    uint16_t yOffset = 0;
    uint16_t addrYExtend = GetAddrYExtend();

    g_multiAccountsReceiveWidgets.qrCodeCont = GuiCreateContainerWithParent(parent, 408, 524 + addrYExtend);
    lv_obj_align(g_multiAccountsReceiveWidgets.qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_multiAccountsReceiveWidgets.qrCodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_multiAccountsReceiveWidgets.qrCodeCont, 24, LV_PART_MAIN);

    yOffset += 36;
    g_multiAccountsReceiveWidgets.qrCode = CreateMultiAccountsReceiveQRCode(g_multiAccountsReceiveWidgets.qrCodeCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(CreateMultiAccountsReceiveQRCode, 420, 420);

    lv_obj_align(g_multiAccountsReceiveWidgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 16;
    g_multiAccountsReceiveWidgets.addressLabel = GuiCreateNoticeLabel(g_multiAccountsReceiveWidgets.qrCodeCont, "");
    uint16_t addressLabelWidth = g_chainCard == HOME_WALLET_CARD_MONERO ? 336 : 280;
    lv_obj_set_width(g_multiAccountsReceiveWidgets.addressLabel, addressLabelWidth);
    lv_obj_align(g_multiAccountsReceiveWidgets.addressLabel, LV_ALIGN_TOP_LEFT, 36, yOffset);

    if (g_chainCard != HOME_WALLET_CARD_MONERO) {
        lv_obj_add_event_cb(g_multiAccountsReceiveWidgets.addressLabel, ShowAddressDetailHandler, LV_EVENT_CLICKED, NULL);
        lv_obj_add_flag(g_multiAccountsReceiveWidgets.addressLabel, LV_OBJ_FLAG_CLICKABLE);
        g_multiAccountsReceiveWidgets.questionImg = GuiCreateImg(g_multiAccountsReceiveWidgets.qrCodeCont, &imgInfo);
        lv_obj_align(g_multiAccountsReceiveWidgets.questionImg, LV_ALIGN_TOP_LEFT, 348, yOffset + 4);
        lv_obj_add_event_cb(g_multiAccountsReceiveWidgets.questionImg, ShowAddressDetailHandler, LV_EVENT_CLICKED, NULL);
        lv_obj_add_flag(g_multiAccountsReceiveWidgets.questionImg, LV_OBJ_FLAG_CLICKABLE);
    }

    yOffset += 60;

    yOffset += 16;
    g_multiAccountsReceiveWidgets.addressCountLabel = GuiCreateIllustrateLabel(g_multiAccountsReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_multiAccountsReceiveWidgets.addressCountLabel, LV_ALIGN_TOP_LEFT, 36, yOffset + addrYExtend);

    g_multiAccountsReceiveWidgets.addressButton = lv_btn_create(g_multiAccountsReceiveWidgets.qrCodeCont);
    lv_obj_set_size(g_multiAccountsReceiveWidgets.addressButton, 336, 36);
    lv_obj_align(g_multiAccountsReceiveWidgets.addressButton, LV_ALIGN_TOP_MID, 0, 464 + addrYExtend);
    lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.addressButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_multiAccountsReceiveWidgets.addressButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_multiAccountsReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_multiAccountsReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    if (IsAddressSwitchable()) {
        lv_obj_add_event_cb(g_multiAccountsReceiveWidgets.addressButton, OpenSwitchAddressHandler, LV_EVENT_CLICKED, NULL);
        tempObj = GuiCreateImg(g_multiAccountsReceiveWidgets.addressButton, &imgArrowRight);
        lv_obj_set_style_img_opa(tempObj, LV_OPA_80, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_CENTER, 150, 0);
    }

    const char *coin = GetCoinCardByIndex(g_chainCard)->coin;
    if (!GetFirstReceive(coin)) {
        char attentionText[BUFFER_SIZE_256];
        GetAttentionText(attentionText);
        g_multiAccountsReceiveWidgets.attentionCont = GuiCreateConfirmHintBox(&imgInformation, _("Attention"), attentionText, NULL, _("got_it"), WHITE_COLOR_OPA20);
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_multiAccountsReceiveWidgets.attentionCont), CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
        SetFirstReceive(coin, true);
    }
}

void GetAttentionText(char *text)
{
    switch (g_chainCard) {
    default:
        snprintf_s(text, BUFFER_SIZE_256, _("receive_coin_hint_fmt"), GetCoinCardByIndex(g_chainCard)->coin);
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
        g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressCountLabel = GuiCreateTextLabel(cont, "");
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 20 + 103 * i);
        g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "");
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, SwitchAddressHandler, LV_EVENT_CLICKED, NULL);

        g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg = GuiCreateImg(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg = GuiCreateImg(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

        index++;
    }
    RefreshSwitchAddress();
}

static void SetCurrentSelectIndex(uint32_t index)
{
    g_selectedIndex[GetCurrentAccountIndex()] = index;
    SetAccountReceiveIndex(GetCoinCardByIndex(g_chainCard)->coin, index);
}

static bool IsIndexSelectChanged()
{
    return g_tmpIndex != g_selectedIndex[GetCurrentAccountIndex()];
}

static void UpdateConfirmIndexBtn(void)
{
    if (IsIndexSelectChanged()) {
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.confirmIndexBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiAccountsReceiveWidgets.confirmIndexBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.confirmIndexBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiAccountsReceiveWidgets.confirmIndexBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}

static bool IsAccountSelectChanged()
{
    return g_tmpAccount != g_selectedAccount[GetCurrentAccountIndex()];
}

static void UpdateConfirmAccountBtn(void)
{
    if (IsAccountSelectChanged()) {
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.confirmAccountBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiAccountsReceiveWidgets.confirmAccountBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.confirmAccountBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiAccountsReceiveWidgets.confirmAccountBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}

static void ConfirmIndexHandler(lv_event_t *e)
{
    if (IsIndexSelectChanged()) {
        SetCurrentSelectIndex(g_tmpIndex);
        ReturnHandler(e);
    }
}

static void ConfirmAccountHandler(lv_event_t *e)
{
    if (IsAccountSelectChanged()) {
        g_selectedAccount[GetCurrentAccountIndex()] = g_tmpAccount;
        SetAccountIndex(GetCoinCardByIndex(g_chainCard)->coin, g_tmpAccount);
        g_tmpIndex = 0;
        SetCurrentSelectIndex(g_tmpIndex);
        CloseSwitchAccountHandler(e);
        RefreshQrCode();
    }
}

static void GuiCreateSwitchAccountButtons(lv_obj_t *parent)
{
    lv_obj_t *btn = GuiCreateImgButton(parent, &imgArrowLeft, 66, LeftSwitchAccountBtnHandler, NULL);
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_t *img = lv_obj_get_child(btn, 0);
    g_multiAccountsReceiveWidgets.leftSwitchAccountBtnImg = img;

    btn = GuiCreateImgButton(parent, &imgArrowRight, 66, RightSwitchAccountBtnHandler, NULL);
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 156, -24);
    img = lv_obj_get_child(btn, 0);
    g_multiAccountsReceiveWidgets.rightSwitchAccountBtnImg = img;

    btn = GuiCreateBtn(parent, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -36, 0);
    lv_obj_add_event_cb(btn, ConfirmAccountHandler, LV_EVENT_CLICKED, NULL);
    g_multiAccountsReceiveWidgets.confirmAccountBtn = btn;

    g_tmpAccount = g_selectedAccount[GetCurrentAccountIndex()];
    g_showAccountIndex = g_tmpAccount / 5 * 5;
}

static void GuiCreateSwitchAddressButtons(lv_obj_t *parent)
{
    lv_obj_t *btn = GuiCreateImgButton(parent, &imgArrowLeft, 66, LeftBtnHandler, NULL);
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_t *img = lv_obj_get_child(btn, 0);
    if (g_showIndex < 5) {
        lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    }
    g_multiAccountsReceiveWidgets.leftBtnImg = img;

    btn = GuiCreateImgButton(parent, &imgArrowRight, 66, RightBtnHandler, NULL);
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 156, -24);
    img = lv_obj_get_child(btn, 0);
    g_multiAccountsReceiveWidgets.rightBtnImg = img;

    btn = GuiCreateBtn(parent, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, ConfirmIndexHandler, LV_EVENT_CLICKED, NULL);
    g_multiAccountsReceiveWidgets.confirmIndexBtn = btn;
    UpdateConfirmIndexBtn();
}

static void RefreshQrCode(void)
{
    AddressDataItem_t addressDataItem;

    ModelGetAddress(GetAccountReceiveIndex(GetCoinCardByIndex(g_chainCard)->coin), &addressDataItem, 0);
    lv_qrcode_update(g_multiAccountsReceiveWidgets.qrCode, addressDataItem.address, strnlen_s(addressDataItem.address, ADDRESS_MAX_LEN));
    lv_obj_t *fullscreen_qrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullscreen_qrcode) {
        lv_qrcode_update(fullscreen_qrcode, addressDataItem.address, strnlen_s(addressDataItem.address, ADDRESS_MAX_LEN));
    }
    char string[128] = {0};
    if (g_chainCard == HOME_WALLET_CARD_MONERO) {
        snprintf_s(string, sizeof(string), "%s", addressDataItem.address);
    } else {
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 20);
    }
    lv_label_set_text(g_multiAccountsReceiveWidgets.addressLabel, string);
    lv_label_set_text_fmt(g_multiAccountsReceiveWidgets.addressCountLabel, "%s-%u", _("Address"), (addressDataItem.index));
}

static void RefreshSwitchAddress(void)
{
    AddressDataItem_t addressDataItem;

    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem, 0);
        lv_label_set_text_fmt(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "%s-%u", _("Address"), (addressDataItem.index));
        char string[128] = {0};
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
        lv_label_set_text(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == g_tmpIndex) {
            lv_obj_add_state(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == GetMaxAddressIndex()) {
            end = true;
        }
        index++;
    }
}

static int GetMaxAddressIndex(void)
{
    return GENERAL_ADDRESS_INDEX_MAX;
}

static int GetMaxAccountIndex(void)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ADA:
        return ACCOUNT_INDEX_MAX;
        break;
    case HOME_WALLET_CARD_MONERO:
        return GENERAL_ACCOUNT_INDEX_MAX;
        break;
    default:
        break;
    }
    return ACCOUNT_INDEX_MAX;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    lv_obj_add_flag(g_multiAccountsReceiveWidgets.attentionCont, LV_OBJ_FLAG_HIDDEN);
}

static void JumpToAccountHandler(lv_event_t *e)
{
    GuiCreateGotoAddressWidgets(g_multiAccountsReceiveWidgets.tileSwitchAddress);
}

static void ShowAddressDetailHandler(lv_event_t *e)
{

    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.moreCont);
    GuiCreateAddressDetailWidgets(g_multiAccountsReceiveWidgets.tileQrCode);
}

static void GuiCreateAddressDetailWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *label, *last;

    if (g_multiAccountsReceiveWidgets.addressDetailCont == NULL) {
        g_multiAccountsReceiveWidgets.addressDetailCont = GuiCreateHintBox(530);
        lv_obj_add_event_cb(lv_obj_get_child(g_multiAccountsReceiveWidgets.addressDetailCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_multiAccountsReceiveWidgets.addressDetailCont);
        cont = g_multiAccountsReceiveWidgets.addressDetailCont;

        label = GuiCreateTextLabel(cont, _("Path"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 24 + 270);
        last = label;

        AddressDataItem_t addressDataItem;
        ModelGetAddress(g_selectedIndex[GetCurrentAccountIndex()], &addressDataItem, 0);

        label = GuiCreateIllustrateLabel(cont, addressDataItem.path);
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        last = label;

        label = GuiCreateTextLabel(cont, _("Address"));
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        last = label;

        label = GuiCreateIllustrateLabel(cont, addressDataItem.address);
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        last = label;

        label = GuiCreateTextLabel(cont, _("receive_ada_enterprise_address"));
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        last = label;

        ModelGetAddress(g_selectedIndex[GetCurrentAccountIndex()], &addressDataItem, 1);
        label = GuiCreateIllustrateLabel(cont, addressDataItem.address);
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        last = label;

        label = GuiCreateTextLabel(cont, _("receive_ada_stake_address"));
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        last = label;

        // we only show stake key at m/1852'/1815'/X'/0/0
        ModelGetAddress(0, &addressDataItem, 2);
        label = GuiCreateIllustrateLabel(cont, addressDataItem.address);
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }
}

static void CloseSwitchAddressHandler(lv_event_t *e)
{

    lv_obj_add_flag(g_multiAccountsReceiveWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
}

static void GuiCreateGotoAddressWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *label, *line, *closeBtn;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    g_inputAccountValid = false;

    if (g_multiAccountsReceiveWidgets.inputAccountCont == NULL) {
        g_multiAccountsReceiveWidgets.inputAccountCont = GuiCreateHintBox(530);
        lv_obj_add_event_cb(lv_obj_get_child(g_multiAccountsReceiveWidgets.inputAccountCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_multiAccountsReceiveWidgets.inputAccountCont);
        cont = g_multiAccountsReceiveWidgets.inputAccountCont;

        label = GuiCreateNoticeLabel(cont, _("receive_btc_receive_change_address_title"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 30 + 270);
        label = GuiCreateNoticeLabel(cont, "");
        lv_label_set_text_fmt(label, "%s-", _("Address"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 108 + 270);
        g_multiAccountsReceiveWidgets.inputAccountLabel = GuiCreateTextLabel(cont, "");
        lv_obj_align(g_multiAccountsReceiveWidgets.inputAccountLabel, LV_ALIGN_TOP_LEFT, 38 + lv_obj_get_self_width(label), 108 + 270);
        label = GuiCreateIllustrateLabel(cont, _("receive_btc_receive_change_address_limit"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 170 + 270);
        lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        g_multiAccountsReceiveWidgets.overflowLabel = label;

        line = GuiCreateLine(cont, points, 2);
        lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 160 + 270);

        lv_obj_t *keyboard = GuiCreateNumKeyboard(cont, InputAddressIndexKeyboardHandler, NUM_KEYBOARD_NORMAL, NULL);
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_add_style(keyboard, &g_enterPressBtnmStyle, LV_STATE_PRESSED | LV_PART_ITEMS);
        lv_btnmatrix_set_btn_ctrl(keyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
        g_multiAccountsReceiveWidgets.inputAccountKeyboard = keyboard;

        closeBtn = GuiCreateImgButton(cont, &imgClose, 40, CloseSwitchAddressHandler, NULL);
        lv_obj_align(closeBtn, LV_ALIGN_TOP_RIGHT, -36, 27 + 270);
    } else {
        lv_label_set_text(g_multiAccountsReceiveWidgets.inputAccountLabel, "");
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_multiAccountsReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

static void InputAddressIndexKeyboardHandler(lv_event_t *e)
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
        strcpy_s(input, sizeof(input), lv_label_get_text(g_multiAccountsReceiveWidgets.inputAccountLabel));
        if (strcmp(txt, LV_SYMBOL_OK) == 0) {
            if (g_inputAccountValid) {
                sscanf(input, "%u", &g_tmpIndex);
                g_showIndex = g_tmpIndex / 5 * 5;
                RefreshSwitchAddress();
                lv_obj_add_flag(g_multiAccountsReceiveWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
                g_inputAccountValid = false;
                UpdateConfirmIndexBtn();
            }
        } else if (strcmp(txt, "-") == 0) {
            len = strlen(input);
            if (len >= 1) {
                input[len - 1] = '\0';
                lv_label_set_text(g_multiAccountsReceiveWidgets.inputAccountLabel, input);
                lv_obj_add_flag(g_multiAccountsReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
                if (strlen(input) >= 1) {
                    g_inputAccountValid = true;
                } else {
                    g_inputAccountValid = false;
                }
            }
        } else if (strlen(input) < 15) {
            strcat(input, txt);
            longInt = strtol(input, NULL, 10);
            if (longInt >= GENERAL_ADDRESS_INDEX_MAX) {
                input[9] = '\0';
                lv_obj_clear_flag(g_multiAccountsReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_multiAccountsReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            }
            if (longInt > 0) {
                if (input[0] == '0') {
                    lv_label_set_text(g_multiAccountsReceiveWidgets.inputAccountLabel, input + 1);
                } else {
                    lv_label_set_text(g_multiAccountsReceiveWidgets.inputAccountLabel, input);
                }
            } else {
                lv_label_set_text(g_multiAccountsReceiveWidgets.inputAccountLabel, "0");
            }
            g_inputAccountValid = true;
        } else {
            g_inputAccountValid = false;
            printf("input to long\r\n");
        }
        SetKeyboardValid(g_inputAccountValid);
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

static void MoreHandler(lv_event_t *e)
{

    if (g_multiAccountsReceiveWidgets.moreCont == NULL) {
        GuiCreateMoreWidgets(g_multiAccountsReceiveWidgets.tileQrCode);
    } else {
        lv_obj_del(g_multiAccountsReceiveWidgets.moreCont);
        g_multiAccountsReceiveWidgets.moreCont = NULL;
    }
}

static void TutorialHandler(lv_event_t *e)
{

    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.moreCont);

    TUTORIAL_LIST_INDEX_ENUM index = TUTORIAL_ADA_RECEIVE;
    if (g_chainCard == HOME_WALLET_CARD_MONERO) {
        index = TUTORIAL_XMR_RECEIVE;
    }
    GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
}

static void SetKeyboardValid(bool validation)
{
    if (validation) {
        if (lv_btnmatrix_has_btn_ctrl(g_multiAccountsReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED)) {
            lv_btnmatrix_clear_btn_ctrl(g_multiAccountsReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
        }
        lv_btnmatrix_set_btn_ctrl(g_multiAccountsReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
    } else {
        if (lv_btnmatrix_has_btn_ctrl(g_multiAccountsReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED)) {
            lv_btnmatrix_clear_btn_ctrl(g_multiAccountsReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
        }
        lv_btnmatrix_set_btn_ctrl(g_multiAccountsReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
    }
}

static void LeftBtnHandler(lv_event_t *e)
{

    lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex >= 5) {
        g_showIndex -= 5;
        RefreshSwitchAddress();
    }
    if (g_showIndex < 5) {
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}

static void RightBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex < GetMaxAddressIndex() - 5) {
        g_showIndex += 5;
        RefreshSwitchAddress();
    }
    if (g_showIndex >= GetMaxAddressIndex() - 5) {
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}


static void LeftSwitchAccountBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightSwitchAccountBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showAccountIndex >= 5) {
        g_showAccountIndex -= 5;
        RefreshSwitchAccount();
    }
    if (g_showAccountIndex < 5) {
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftSwitchAccountBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}

static void RightSwitchAccountBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftSwitchAccountBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showAccountIndex < GetMaxAccountIndex() - 5) {
        g_showAccountIndex += 5;
        RefreshSwitchAccount();
    }
    if (g_showAccountIndex >= GetMaxAccountIndex() - 5) {
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightSwitchAccountBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}

static bool IsAddressSwitchable()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_MONERO:
    case HOME_WALLET_CARD_ADA:
        return true;
    default:
        return false;
    }
}

static bool IsPathTypeSwitchable()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ADA:
        return true;
    default:
        return false;
    }
}

static bool HasMoreBtn()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_MONERO:
    case HOME_WALLET_CARD_ADA:
        return true;

    default:
        return false;
    }
}

static void SwitchAddressHandler(lv_event_t *e)
{
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < 5; i++) {
        if (checkBox == g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox) {
            lv_obj_add_state(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            g_tmpIndex = g_showIndex + i;
        } else {
            lv_obj_clear_state(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
    }
    UpdateConfirmIndexBtn();
}

static void SwitchAccountHandler(lv_event_t *e)
{
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < 5; i++) {
        if (checkBox == g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox) {
            lv_obj_add_state(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            g_tmpAccount = g_showAccountIndex + i;
        } else {
            lv_obj_clear_state(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
    }
    UpdateConfirmAccountBtn();
}

static void OpenSwitchAddressHandler(lv_event_t *e)
{
    GuiMultiAccountsReceiveGotoTile(RECEIVE_TILE_SWITCH_ACCOUNT);
    RefreshSwitchAddress();
}

static void OpenChangePathTypeHandler(lv_event_t *e)
{
    GuiMultiAccountsReceiveGotoTile(RECEIVE_TILE_SWITCH_PATH_TYPE);
    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.moreCont);
}

static void OpenSwitchAccountHandler(lv_event_t *e)
{
    // g_tmpAccount = g_selectedAccount[GetCurrentAccountIndex()];
    g_tmpAccount = GetAccountIndex(GetCoinCardByIndex(g_chainCard)->coin);
    GuiCreateSwitchAccountWidget();
    GUI_DEL_OBJ(g_multiAccountsReceiveWidgets.moreCont);
}

static void CloseSwitchAccountHandler(lv_event_t *e)
{
    GUI_PAGE_DEL(g_multiAccountsReceiveWidgets.switchAccountCont);
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;

    uint32_t index = g_showAccountIndex;

    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        if (g_chainCard == HOME_WALLET_CARD_MONERO) {
            ModelGetAddress(index, &addressDataItem, 1);
            lv_label_set_text_fmt(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressCountLabel, "%s-%u", _("Account"), (addressDataItem.index));
            char string[128] = {0};
            CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
            lv_label_set_text(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressLabel, string);
        } else {
            char temp[BUFFER_SIZE_64];
            snprintf_s(temp, BUFFER_SIZE_64, "m/1852'/1815'/%u'", index);
            lv_label_set_text(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressLabel, temp);
            lv_label_set_text_fmt(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressCountLabel, "%s-%u", _("Account"), index);
        }
        if (end) {
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == g_tmpAccount) {
            lv_obj_add_state(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == GetMaxAccountIndex()) {
            end = true;
        }
        index++;
    }

    if (g_showAccountIndex < 5) {
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftSwitchAccountBtnImg, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightSwitchAccountBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    } else if (g_showAccountIndex >= GetMaxAccountIndex() - 5) {
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftSwitchAccountBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightSwitchAccountBtnImg, LV_OPA_30, LV_PART_MAIN);
    } else {
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.leftSwitchAccountBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_img_opa(g_multiAccountsReceiveWidgets.rightSwitchAccountBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    }
}

static void GuiCreateSwitchAccountWidget()
{
    PageWidget_t *page = CreatePageWidget();
    g_multiAccountsReceiveWidgets.switchAccountCont = page;
    SetNavBarLeftBtn(page->navBarWidget, NVS_BAR_RETURN, CloseSwitchAccountHandler, NULL);
    SetMidBtnLabel(page->navBarWidget, NVS_BAR_MID_LABEL, _("switch_account"));
    // Create the account list page.
    uint32_t index;
    lv_obj_t *cont = GuiCreateContainerWithParent(page->contentZone, 408, 514);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *line, *label, *checkBox, *checkedImg, *uncheckedImg, *address;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    index = 0;
    for (uint32_t i = 0; i < 5; i++) {
        g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressCountLabel = GuiCreateTextLabel(cont, "");
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 20 + 103 * i);
        g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "");
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);

        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, SwitchAccountHandler, LV_EVENT_CLICKED, NULL);

        g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg = GuiCreateImg(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg = GuiCreateImg(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_multiAccountsReceiveWidgets.switchAccountWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

        index++;
    }

    lv_obj_t *tmCont = GuiCreateContainerWithParent(page->contentZone, 480, 114);
    lv_obj_align(tmCont, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(tmCont, BLACK_COLOR, LV_PART_MAIN);
    GuiCreateSwitchAccountButtons(tmCont);
    RefreshSwitchAccount();

    UpdateConfirmAccountBtn();
}

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item, uint8_t type)
{
    char *xPub = NULL, *pvk = NULL, hdPath[BUFFER_SIZE_128];
    SimpleResponse_c_char *result = NULL;
    uint32_t currentAccount = GetAccountIndex(GetCoinCardByIndex(g_chainCard)->coin);;
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ADA:
        xPub = GetCurrentAccountPublicKey(GetReceivePageAdaXPubTypeByIndex(currentAccount));
        snprintf_s(hdPath, BUFFER_SIZE_128, "m/1852'/1815'/%u'/0/%u", currentAccount, index);
        // cardano mainnet;
        switch (type) {
        case 1:
            result = cardano_get_enterprise_address(xPub, index, 1);
            break;
        case 2:
            result = cardano_get_stake_address(xPub, index, 1);
            break;
        default:
            result = cardano_get_base_address(xPub, index, 1);
            break;
        }
        break;
    case HOME_WALLET_CARD_MONERO:
        switch (type) {
        case 1:
            xPub = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_0);
            pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
            snprintf_s(hdPath, BUFFER_SIZE_16, "");
            bool isPrimaryAccount = index == 0;
            result = monero_get_address(xPub, pvk, index, 0, !isPrimaryAccount);
            break;
        default:
            xPub = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_0);
            pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
            uint32_t accountIndex = g_selectedAccount[GetCurrentAccountIndex()];
            bool isSubAddress = index != 0 || accountIndex != 0;
            snprintf_s(hdPath, BUFFER_SIZE_128, "m/44'/128'/0'/0/%u", index);
            result = monero_get_address(xPub, pvk, accountIndex, index, isSubAddress);
            break;
        }
        break;
    default:
        break;
    }
    ASSERT(xPub);

    if (result->error_code == 0) {
        item->index = index;
        strcpy_s(item->address, ADDRESS_MAX_LEN, result->data);
        strcpy_s(item->path, PATH_ITEM_MAX_LEN, hdPath);
    }
    free_simple_response_c_char(result);
}

void GuiResetCurrentMultiAccountsCache(uint8_t index)
{
    if (index > 2) {
        return;
    }
    g_selectedIndex[index] = 0;
    g_selectedAccount[index] = 0;
}
#endif
