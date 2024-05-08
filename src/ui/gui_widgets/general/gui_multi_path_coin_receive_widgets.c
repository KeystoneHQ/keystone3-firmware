#ifndef BTC_ONLY
#include "gui_multi_path_coin_receive_widgets.h"
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
#include "gui_tutorial_widgets.h"
#include "gui_fullscreen_mode.h"
#include "keystore.h"
#include "gui_page.h"
#include "account_manager.h"
#include "gui_home_widgets.h"
#include "gui_global_resources.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

#define GENERAL_ADDRESS_INDEX_MAX                       (999999999)
#define ETH_LEDGER_LIVE_ADDRESS_INDEX_MAX               (9)
#define SOL_BIP44_ADDRESS_INDEX_MAX                     (9)
#define SOL_BIP44_ROOT_ADDRESS_INDEX_MAX                (0)
#define SOL_BIP44_CHANGE_ADDRESS_INDEX_MAX              (9)

typedef enum {
    RECEIVE_TILE_QRCODE = 0,
    RECEIVE_TILE_SWITCH_ACCOUNT,
    RECEIVE_TILE_CHANGE_PATH,

    RECEIVE_TILE_BUTT,
} EthereumReceiveTile;

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
    lv_obj_t *confirmAddrTypeBtn;
    lv_obj_t *confirmAddrIndexBtn;
    lv_obj_t *inputAddressLabel;
    lv_obj_t *overflowLabel;
    PathWidgetsItem_t changePathWidgets[3];
    SwitchAddressWidgetsItem_t switchAddressWidgets[5];
} MultiPathCoinReceiveWidgets_t;

typedef struct {
    GuiChainCoinType type;
    char tittle[PATH_ITEM_MAX_LEN];
} TittleItem_t;

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
static void GuiEthereumReceiveGotoTile(EthereumReceiveTile tile);
static void GuiCreateQrCodeWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressButtons(lv_obj_t *parent);
static void GuiCreateChangePathWidget(lv_obj_t *parent);

static void RefreshQrCode(void);
static void RefreshSwitchAccount(void);
static void RefreshDefaultAddress(void);
static int GetMaxAddressIndex(void);

static void CloseAttentionHandler(lv_event_t *e);
static void MoreHandler(lv_event_t *e);
static void ChangePathHandler(lv_event_t *e);
static void TutorialHandler(lv_event_t *e);
static void LeftBtnHandler(lv_event_t *e);
static void RightBtnHandler(lv_event_t* e);
static void ChangePathCheckHandler(lv_event_t *e);
static void SwitchAddressHandler(lv_event_t *e);
static void OpenSwitchAddressHandler(lv_event_t *e);

static void GetPathItemSubTitle(char* subTitle, int index, uint32_t maxLen);
static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item);
static void GetEthHdPath(char *hdPath, int index, uint32_t maxLen);
static void GetSolHdPath(char *hdPath, int index, uint32_t maxLen);
static void GetEthRootPath(char *rootPath, int index, uint32_t maxLen);
static char *GetEthXpub(int index);
static char *GetSolXpub(int index);

static uint32_t GetPathIndex(void);
static void SetPathIndex(uint32_t index);

static int GetSOLMaxAddressIndex(void);
static int GetEthMaxAddressIndex(void);

static const char* GetChangePathItemTitle(uint32_t i);
static void GetChangePathLabelHint(char* hint);

static void SetCurrentSelectIndex(uint32_t selectIndex);
static uint32_t GetCurrentSelectIndex();
static void GetHint(char *hint);
static void ModelGetSolAddress(uint32_t index, AddressDataItem_t *item);
static void ModelGetAddress(uint32_t index, AddressDataItem_t *item);
static void GetSolPathItemSubTitle(char* subTitle, int index, uint32_t maxLen);
static void UpdateConfirmAddrIndexBtn(void);
static void UpdateConfirmAddrTypeBtn(void);
static void UpdateAddrTypeCheckbox(uint8_t i, bool isChecked);
static bool IsOnlyOneAddress(uint8_t addrType);

static MultiPathCoinReceiveWidgets_t g_multiPathCoinReceiveWidgets;
static EthereumReceiveTile g_multiPathCoinReceiveTileNow;

static const PathItem_t g_ethPaths[] = {
    {"BIP44 Standard",          "",     "m/44'/60'/0'"  },
    {"Ledger Live",             "",     "m/44'/60'"     },
    {"Ledger Legacy",           "",     "m/44'/60'/0'"  },
};
static const PathItem_t g_solPaths[] = {
    {"Account-based Path",      "",     "m/44'/501'"  },
    {"Single Account Path",     "",     "m/44'/501'"  },
    {"Sub-account Path",        "",     "m/44'/501'"  },
};
static lv_obj_t *g_addressLabel[2];
static lv_obj_t *g_goToAddressIcon;

static uint32_t g_showIndex;
static uint32_t g_selectIndex;
static uint32_t g_selectType = 0;
static uint8_t g_currentAccountIndex = 0;
static uint32_t g_ethSelectIndex[3] = {0};
static uint32_t g_solSelectIndex[3] = {0};
static uint32_t g_ethPathIndex[3] = {0};
static uint32_t g_solPathIndex[3] = {0};
static PageWidget_t *g_pageWidget;
static HOME_WALLET_CARD_ENUM g_chainCard;
static lv_obj_t *g_derivationPathDescLabel = NULL;
static char **g_derivationPathDescs = NULL;
static lv_obj_t *g_egCont = NULL;

static void InitDerivationPathDesc(uint8_t chain)
{
    switch (chain) {
    case HOME_WALLET_CARD_ETH:
        g_derivationPathDescs = GetDerivationPathDescs(ETH_DERIVATION_PATH_DESC);
        break;
    case HOME_WALLET_CARD_SOL:
        g_derivationPathDescs = GetDerivationPathDescs(SOL_DERIVATION_PATH_DESC);
        break;
    default:
        break;
    }
}

void GuiMultiPathCoinReceiveInit(uint8_t chain)
{
    InitDerivationPathDesc(chain);
    g_chainCard = chain;
    g_currentAccountIndex = GetCurrentAccountIndex();
    g_selectIndex = GetCurrentSelectIndex();
    g_selectType = GetPathIndex();

    g_pageWidget = CreatePageWidget();
    g_multiPathCoinReceiveWidgets.cont = g_pageWidget->contentZone;
    g_multiPathCoinReceiveWidgets.tileView = lv_tileview_create(g_multiPathCoinReceiveWidgets.cont);
    lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_multiPathCoinReceiveWidgets.tileQrCode = lv_tileview_add_tile(g_multiPathCoinReceiveWidgets.tileView, RECEIVE_TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_multiPathCoinReceiveWidgets.tileQrCode);
    g_multiPathCoinReceiveWidgets.tileSwitchAccount = lv_tileview_add_tile(g_multiPathCoinReceiveWidgets.tileView, RECEIVE_TILE_SWITCH_ACCOUNT, 0, LV_DIR_HOR);
    GuiCreateSwitchAddressWidget(g_multiPathCoinReceiveWidgets.tileSwitchAccount);
    GuiCreateSwitchAddressButtons(g_multiPathCoinReceiveWidgets.tileSwitchAccount);
    g_multiPathCoinReceiveWidgets.tileChangePath = lv_tileview_add_tile(g_multiPathCoinReceiveWidgets.tileView, RECEIVE_TILE_CHANGE_PATH, 0, LV_DIR_HOR);
    GuiCreateChangePathWidget(g_multiPathCoinReceiveWidgets.tileChangePath);
    lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.tileView, LV_OBJ_FLAG_SCROLLABLE);

    GuiMultiPathCoinReceiveRefresh();
}

void GuiMultiPathCoinReceiveDeInit(void)
{
    GUI_DEL_OBJ(g_multiPathCoinReceiveWidgets.moreCont)
    GUI_DEL_OBJ(g_multiPathCoinReceiveWidgets.attentionCont)
    GUI_DEL_OBJ(g_multiPathCoinReceiveWidgets.cont)

    CLEAR_OBJECT(g_multiPathCoinReceiveWidgets);
    g_multiPathCoinReceiveTileNow = 0;
    GuiFullscreenModeCleanUp();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiMultiPathCoinReceiveRefresh(void)
{
    char walletTitle[BUFFER_SIZE_32];
    switch (g_multiPathCoinReceiveTileNow) {
    case RECEIVE_TILE_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        switch (g_chainCard) {
        case HOME_WALLET_CARD_ETH:
            SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_ETH, _("receive_eth_receive_main_title"));
            break;
        case HOME_WALLET_CARD_SOL:
            snprintf_s(walletTitle, BUFFER_SIZE_32, _("receive_coin_fmt"), "SOL");
            SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_SOL, walletTitle);
            break;
        default:
            break;
        }
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, MoreHandler, NULL);
        RefreshQrCode();
        if (IsOnlyOneAddress(GetPathIndex())) {
            lv_obj_add_flag(g_goToAddressIcon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(g_goToAddressIcon, LV_OBJ_FLAG_HIDDEN);
        }
        break;
    case RECEIVE_TILE_SWITCH_ACCOUNT:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("switch_account"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        g_selectIndex = GetCurrentSelectIndex();
        g_showIndex = g_selectIndex / 5 * 5;
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        } else if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        } else {
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        }
        UpdateConfirmAddrIndexBtn();
        break;
    case RECEIVE_TILE_CHANGE_PATH:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("derivation_path_change"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        g_selectType = GetPathIndex();
        for (uint32_t i = 0; i < 3; i++) {
            UpdateAddrTypeCheckbox(i, g_selectType == i);
        }
        UpdateConfirmAddrTypeBtn();
        break;
    default:
        break;
    }
}

void GuiMultiPathCoinReceivePrevTile(void)
{
    GuiEthereumReceiveGotoTile(RECEIVE_TILE_QRCODE);
}

static void GuiCreateMoreWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *btn, *img, *label;

    g_multiPathCoinReceiveWidgets.moreCont = GuiCreateHintBox(228);
    lv_obj_add_event_cb(lv_obj_get_child(g_multiPathCoinReceiveWidgets.moreCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_multiPathCoinReceiveWidgets.moreCont);
    cont = g_multiPathCoinReceiveWidgets.moreCont;

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 456, 84);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 24 + 572);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, ChangePathHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(btn, &imgPath);
    lv_obj_align(img, LV_ALIGN_CENTER, -186, 0);
    label = GuiCreateTextLabel(btn, _("derivation_path_change"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);

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
    label = GuiCreateTextLabel(btn, _("Tutorial"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);
}

static void GuiEthereumReceiveGotoTile(EthereumReceiveTile tile)
{
    g_multiPathCoinReceiveTileNow = tile;
    GuiMultiPathCoinReceiveRefresh();
    lv_obj_set_tile_id(g_multiPathCoinReceiveWidgets.tileView, g_multiPathCoinReceiveTileNow, 0, LV_ANIM_OFF);
}

lv_obj_t* CreateEthereumReceiveQRCode(lv_obj_t* parent, uint16_t w, uint16_t h)
{
    lv_obj_t* qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    return qrcode;
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *tempObj;
    uint16_t yOffset = 0;

    g_multiPathCoinReceiveWidgets.qrCodeCont = GuiCreateContainerWithParent(parent, 408, 524);
    lv_obj_align(g_multiPathCoinReceiveWidgets.qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_multiPathCoinReceiveWidgets.qrCodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_multiPathCoinReceiveWidgets.qrCodeCont, 24, LV_PART_MAIN);

    yOffset += 36;
    g_multiPathCoinReceiveWidgets.qrCode = CreateEthereumReceiveQRCode(g_multiPathCoinReceiveWidgets.qrCodeCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(CreateEthereumReceiveQRCode, 420, 420);
    lv_obj_align(g_multiPathCoinReceiveWidgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 16;
    g_multiPathCoinReceiveWidgets.addressLabel = GuiCreateNoticeLabel(g_multiPathCoinReceiveWidgets.qrCodeCont, "");
    lv_obj_set_width(g_multiPathCoinReceiveWidgets.addressLabel, 336);
    lv_obj_align(g_multiPathCoinReceiveWidgets.addressLabel, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 60;

    yOffset += 16;
    g_multiPathCoinReceiveWidgets.addressCountLabel = GuiCreateIllustrateLabel(g_multiPathCoinReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_multiPathCoinReceiveWidgets.addressCountLabel, LV_ALIGN_TOP_LEFT, 36, yOffset);

    g_multiPathCoinReceiveWidgets.addressButton = lv_btn_create(g_multiPathCoinReceiveWidgets.qrCodeCont);
    lv_obj_set_size(g_multiPathCoinReceiveWidgets.addressButton, 336, 36);
    lv_obj_align(g_multiPathCoinReceiveWidgets.addressButton, LV_ALIGN_TOP_MID, 0, 464);
    lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.addressButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_multiPathCoinReceiveWidgets.addressButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_multiPathCoinReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_multiPathCoinReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(g_multiPathCoinReceiveWidgets.addressButton, OpenSwitchAddressHandler, LV_EVENT_CLICKED, NULL);
    tempObj = GuiCreateImg(g_multiPathCoinReceiveWidgets.addressButton, &imgArrowRight);
    lv_obj_set_style_img_opa(tempObj, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(tempObj, LV_ALIGN_CENTER, 150, 0);
    g_goToAddressIcon = tempObj;
    const char* coin = GetCoinCardByIndex(g_chainCard)->coin;

    if (!GetFirstReceive(coin)) {
        g_multiPathCoinReceiveWidgets.attentionCont = GuiCreateHintBox(386);
        tempObj = GuiCreateImg(g_multiPathCoinReceiveWidgets.attentionCont, &imgInformation);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 462);
        tempObj = GuiCreateLittleTitleLabel(g_multiPathCoinReceiveWidgets.attentionCont, _("Attention"));
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 558);
        char hint[BUFFER_SIZE_256];
        GetHint(hint);
        tempObj = GuiCreateIllustrateLabel(g_multiPathCoinReceiveWidgets.attentionCont, hint);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 610);
        tempObj = GuiCreateTextBtn(g_multiPathCoinReceiveWidgets.attentionCont, _("got_it"));
        lv_obj_set_size(tempObj, 122, 66);
        lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
        SetFirstReceive(coin, true);
    }
}

static void GetHint(char *hint)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        strcpy_s(hint, BUFFER_SIZE_256, _("receive_eth_alert_desc"));
        break;
    case HOME_WALLET_CARD_SOL:
        snprintf_s(hint, BUFFER_SIZE_256, _("receive_coin_hint_fmt"), "SOL");
        break;
    default:
        break;
    }
}

static void GuiCreateSwitchAddressWidget(lv_obj_t *parent)
{
    //Create the account list page.
    uint32_t index;
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 514);
    lv_obj_t *line;
    static lv_point_t points[2] =  {{0, 0}, {360, 0}};
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    index = 0;
    for (uint32_t i = 0; i < 5; i++) {
        g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel = GuiCreateTextLabel(cont, "");
        lv_obj_align(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 20 + 103 * i);
        g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "");
        lv_obj_align(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, SwitchAddressHandler, LV_EVENT_CLICKED, NULL);

        g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg = GuiCreateImg(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg = GuiCreateImg(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

        index++;
    }
    RefreshSwitchAccount();
}

static bool IsAddrIndexSelectChanged()
{
    return g_selectIndex != GetCurrentSelectIndex();
}

static void UpdateConfirmAddrIndexBtn(void)
{
    if (IsAddrIndexSelectChanged()) {
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.confirmAddrIndexBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiPathCoinReceiveWidgets.confirmAddrIndexBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.confirmAddrIndexBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiPathCoinReceiveWidgets.confirmAddrIndexBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}

static bool IsAddrTypeSelectChanged()
{
    return g_selectType != GetPathIndex();
}

static void UpdateConfirmAddrTypeBtn(void)
{
    if (IsAddrTypeSelectChanged()) {
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.confirmAddrTypeBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiPathCoinReceiveWidgets.confirmAddrTypeBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.confirmAddrTypeBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_multiPathCoinReceiveWidgets.confirmAddrTypeBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}

static void ConfirmAddrIndexHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED && IsAddrIndexSelectChanged()) {
        SetCurrentSelectIndex(g_selectIndex);
        ReturnHandler(e);
    }
}

static void ConfirmAddrTypeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED && IsAddrTypeSelectChanged()) {
        SetPathIndex(g_selectType);
        g_selectIndex = 0;
        SetCurrentSelectIndex(g_selectIndex);
        ReturnHandler(e);
    }
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
    g_multiPathCoinReceiveWidgets.leftBtnImg = img;

    btn = GuiCreateBtn(parent, "");
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 156, -24);
    img = GuiCreateImg(btn, &imgArrowRight);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, RightBtnHandler, LV_EVENT_CLICKED, NULL);
    g_multiPathCoinReceiveWidgets.rightBtnImg = img;

    btn = GuiCreateBtn(parent, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, ConfirmAddrIndexHandler, LV_EVENT_CLICKED, NULL);
    g_multiPathCoinReceiveWidgets.confirmAddrIndexBtn = btn;
    UpdateConfirmAddrIndexBtn();
}

static void GetChangePathLabelHint(char* hint)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        snprintf_s(hint, BUFFER_SIZE_128, _("derivation_path_select_eth"));
        return;
    case HOME_WALLET_CARD_SOL:
        snprintf_s(hint, BUFFER_SIZE_128, _("derivation_path_select_sol"));
        return;
    default:
        break;
    }
}

static const char* GetChangePathItemTitle(uint32_t i)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        return (char *)g_ethPaths[i].title;
    case HOME_WALLET_CARD_SOL:
        if (i == 0) {
            return _("receive_sol_more_t_base_path");
        } else if (i == 1) {
            return _("receive_sol_more_t_single_path");
        } else if (i == 2) {
            return _("receive_sol_more_t_sub_path");
        }
    default:
        break;
    }
    return "";
}

static void ShowEgAddressCont(lv_obj_t *egCont)
{

    if (egCont == NULL) {
        printf("egCont is NULL, cannot show eg address\n");
        return;
    }

    lv_obj_clean(egCont);

    lv_obj_t *prevLabel, *label;

    int egContHeight = 12;
    label = GuiCreateNoticeLabel(egCont, g_derivationPathDescs[g_selectType]);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    lv_obj_update_layout(label);
    egContHeight += lv_obj_get_height(label);
    g_derivationPathDescLabel = label;
    prevLabel = label;

    int gap = 4;
    if (strnlen_s(g_derivationPathDescs[g_selectType], ADDRESS_MAX_LEN) == 0) {
        egContHeight -= lv_obj_get_height(label);
        lv_obj_set_height(label, 0);
        gap = 0;
    }

    const char *desc = _("derivation_path_address_eg");
    label = GuiCreateNoticeLabel(egCont, desc);
    lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, gap);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(label);
    egContHeight =  egContHeight + 4 + lv_obj_get_height(label);
    prevLabel = label;

    lv_obj_t *index = GuiCreateNoticeLabel(egCont, _("0"));
    lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(index);
    egContHeight =  egContHeight + 4 + lv_obj_get_height(index);
    prevLabel = index;

    label = GuiCreateIllustrateLabel(egCont, "");
    lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    g_addressLabel[0] = label;

    if (!IsOnlyOneAddress(g_selectType)) {
        index = GuiCreateNoticeLabel(egCont, _("1"));
        lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(index);
        egContHeight =  egContHeight + 4 + lv_obj_get_height(index);
        prevLabel = index;

        label = GuiCreateIllustrateLabel(egCont, "");
        lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
        g_addressLabel[1] = label;
    }
    egContHeight += 12;
    lv_obj_set_height(egCont, egContHeight);

    RefreshDefaultAddress();
}

static void GuiCreateChangePathWidget(lv_obj_t *parent)
{
    lv_obj_t *cont, *line, *label;
    static lv_point_t points[2] =  {{0, 0}, {360, 0}};
    char string[BUFFER_SIZE_64];
    char lableText[BUFFER_SIZE_128] = {0};
    GetChangePathLabelHint(lableText);
    lv_obj_t *scrollCont = GuiCreateContainerWithParent(parent, 408, 542);
    lv_obj_align(scrollCont, LV_ALIGN_DEFAULT, 36, 0);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *labelHint = GuiCreateIllustrateLabel(scrollCont, lableText);
    lv_obj_set_style_text_opa(labelHint, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 0, 0);

    cont = GuiCreateContainerWithParent(scrollCont, 408, 308);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);

    for (uint32_t i = 0; i < 3; i++) {
        label = GuiCreateTextLabel(cont, GetChangePathItemTitle(i));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 26 + 103 * i);
        GetPathItemSubTitle(string, i, sizeof(string));
        label = GuiCreateLabelWithFontAndTextColor(cont, string, g_defIllustrateFont, 0x919191);
        lv_label_set_recolor(label, true);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i < 2) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * (i + 1));
        }
        g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, ChangePathCheckHandler, LV_EVENT_CLICKED, NULL);

        g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkedImg = GuiCreateImg(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_multiPathCoinReceiveWidgets.changePathWidgets[i].uncheckedImg = GuiCreateImg(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_multiPathCoinReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[g_selectType].checkedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[g_selectType].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *egCont = GuiCreateContainerWithParent(scrollCont, 408, 186);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);
    g_egCont = egCont;
    ShowEgAddressCont(g_egCont);

    lv_obj_t *tmCont = GuiCreateContainerWithParent(parent, 480, 114);
    lv_obj_align(tmCont, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(tmCont, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_t *btn = GuiCreateBtn(tmCont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -36, 0);
    lv_obj_add_event_cb(btn, ConfirmAddrTypeHandler, LV_EVENT_CLICKED, NULL);
    g_multiPathCoinReceiveWidgets.confirmAddrTypeBtn = btn;
    UpdateConfirmAddrTypeBtn();
}

static void RefreshQrCode(void)
{
    AddressDataItem_t addressDataItem;
    ModelGetAddress(GetCurrentSelectIndex(), &addressDataItem);
    lv_qrcode_update(g_multiPathCoinReceiveWidgets.qrCode, addressDataItem.address, strnlen_s(addressDataItem.address, ADDRESS_MAX_LEN));
    lv_obj_t *fullscreen_qrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullscreen_qrcode) {
        lv_qrcode_update(fullscreen_qrcode, addressDataItem.address, strnlen_s(addressDataItem.address, ADDRESS_MAX_LEN));
    }
    lv_label_set_text(g_multiPathCoinReceiveWidgets.addressLabel, addressDataItem.address);
    lv_label_set_text_fmt(g_multiPathCoinReceiveWidgets.addressCountLabel, "%s-%u", _("account_head"), (addressDataItem.index + 1));
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "%s-%u", _("account_head"), (addressDataItem.index + 1));
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
        lv_label_set_text(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == g_selectIndex) {
            lv_obj_add_state(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == GetMaxAddressIndex()) {
            end = true;
        }
        index++;
    }
}

static bool IsOnlyOneAddress(uint8_t addrType)
{
    if (g_chainCard == HOME_WALLET_CARD_SOL && addrType == 1) {
        return true;
    }
    return false;
}

static void RefreshDefaultAddress(void)
{
    char string[128];

    AddressDataItem_t addressDataItem;

    ModelGetAddress(0, &addressDataItem);
    CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
    lv_label_set_text(g_addressLabel[0], string);

    if (!IsOnlyOneAddress(g_selectType)) {
        ModelGetAddress(1, &addressDataItem);
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
        lv_label_set_text(g_addressLabel[1], string);
    }
}

static int GetEthMaxAddressIndex(void)
{
    switch (g_ethPathIndex[GetCurrentAccountIndex()]) {
    case 0:
    case 2:
        return GENERAL_ADDRESS_INDEX_MAX;
    case 1:
        return ETH_LEDGER_LIVE_ADDRESS_INDEX_MAX;
    default:
        break;
    }
    return GENERAL_ADDRESS_INDEX_MAX;
}

static int GetSOLMaxAddressIndex(void)
{
    switch (g_solPathIndex[GetCurrentAccountIndex()]) {
    case 0:
    case 2:
        return SOL_BIP44_ADDRESS_INDEX_MAX;
    case 1:
        return SOL_BIP44_ROOT_ADDRESS_INDEX_MAX;
    default:
        break;
    }
    return GENERAL_ADDRESS_INDEX_MAX;
}

static int GetMaxAddressIndex(void)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        return GetEthMaxAddressIndex();
    case HOME_WALLET_CARD_SOL:
        return GetSOLMaxAddressIndex();
    default:
        break;
    }

    return -1;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    lv_obj_add_flag(g_multiPathCoinReceiveWidgets.attentionCont, LV_OBJ_FLAG_HIDDEN);
}

static void MoreHandler(lv_event_t *e)
{
    if (g_multiPathCoinReceiveWidgets.moreCont == NULL) {
        GuiCreateMoreWidgets(g_multiPathCoinReceiveWidgets.tileQrCode);
    } else {
        lv_obj_del(g_multiPathCoinReceiveWidgets.moreCont);
        g_multiPathCoinReceiveWidgets.moreCont = NULL;
    }
}

static void ChangePathHandler(lv_event_t *e)
{
    if (g_multiPathCoinReceiveWidgets.moreCont != NULL) {
        lv_obj_del(g_multiPathCoinReceiveWidgets.moreCont);
        g_multiPathCoinReceiveWidgets.moreCont = NULL;
    }
    GuiEthereumReceiveGotoTile(RECEIVE_TILE_CHANGE_PATH);
}

static TUTORIAL_LIST_INDEX_ENUM GetTutorialIndex()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        return TUTORIAL_ETH_RECEIVE;
    case HOME_WALLET_CARD_SOL:
        return TUTORIAL_SOL_RECEIVE;
    default:
        break;
    }

    return TUTORIAL_LIST_INDEX_BUTT;
}

static void TutorialHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_multiPathCoinReceiveWidgets.moreCont);

    TUTORIAL_LIST_INDEX_ENUM index = GetTutorialIndex();
    GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
}

static void LeftBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex >= 5) {
        g_showIndex -= 5;
        RefreshSwitchAccount();
    }
    if (g_showIndex < 5) {
        lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}

static void RightBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex < GetMaxAddressIndex() - 5) {
        g_showIndex += 5;
        RefreshSwitchAccount();
    }
    if (g_showIndex >= GetMaxAddressIndex() - 5) {
        lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}

static uint32_t GetPathIndex(void)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        return g_ethPathIndex[g_currentAccountIndex];
        break;
    case HOME_WALLET_CARD_SOL:
        return g_solPathIndex[g_currentAccountIndex];
        break;
    default:
        break;
    }

    return -1;
}

static void SetPathIndex(uint32_t index)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        g_ethPathIndex[g_currentAccountIndex] = index;
        break;
    case HOME_WALLET_CARD_SOL:
        g_solPathIndex[g_currentAccountIndex] = index;
        break;
    default:
        break;
    }
}

static void UpdateAddrTypeCheckbox(uint8_t i, bool isChecked)
{
    if (isChecked) {
        lv_obj_add_state(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_STATE_CHECKED);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        g_selectType = i;
        ShowEgAddressCont(g_egCont);
    } else {
        lv_obj_clear_state(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_STATE_CHECKED);
        lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ChangePathCheckHandler(lv_event_t *e)
{
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < 3; i++) {
        UpdateAddrTypeCheckbox(i, checkBox == g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox);
    }
    UpdateConfirmAddrTypeBtn();
}

static void SwitchAddressHandler(lv_event_t *e)
{
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < 5; i++) {
        if (checkBox == g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox) {
            lv_obj_add_state(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            g_selectIndex = g_showIndex + i;
        } else {
            lv_obj_clear_state(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
    }
    UpdateConfirmAddrIndexBtn();
}

static void OpenSwitchAddressHandler(lv_event_t *e)
{
    if (GetMaxAddressIndex() == 0) {
        return;
    }
    GuiEthereumReceiveGotoTile(RECEIVE_TILE_SWITCH_ACCOUNT);
    RefreshSwitchAccount();
}

static void GetEthPathItemSubTittle(char* subTitle, int index, uint32_t maxLen)
{
    switch (index) {
    case 0:
        strcpy_s(subTitle, maxLen, "m/44'/60'/0'/0/#F5870A X#");
        break;
    case 1:
        strcpy_s(subTitle, maxLen, "m/44'/60'/#F5870A X#'/0/0");
        break;
    case 2:
        strcpy_s(subTitle, maxLen, "m/44'/60'/0'/#F5870A X#");
        break;
    default:
        break;
    }
}

static void GetSolPathItemSubTitle(char* subTitle, int index, uint32_t maxLen)
{
    switch (index) {
    case 0:
        snprintf_s(subTitle, maxLen, "m/44'/501'/#F5870A X#'");
        break;
    case 1:
        snprintf_s(subTitle, maxLen, "m/44'/501'");
        break;
    case 2:
        snprintf_s(subTitle, maxLen, "m/44'/501'/#F5870A X#'/0'");
        break;
    default:
        break;
    }
}

static void GetPathItemSubTitle(char* subTitle, int index, uint32_t maxLen)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        GetEthPathItemSubTittle(subTitle, index, maxLen);
        break;
    case HOME_WALLET_CARD_SOL:
        GetSolPathItemSubTitle(subTitle, index, maxLen);
        break;
    default:
        break;
    }
}

static void GetSolHdPath(char *hdPath, int index, uint32_t maxLen)
{
    uint8_t i = g_solPathIndex[g_currentAccountIndex];
    if (g_multiPathCoinReceiveTileNow == RECEIVE_TILE_CHANGE_PATH) {
        i = g_selectType;
    }
    switch (i) {
    case 0:
        snprintf_s(hdPath, maxLen, "%s/%u'", g_solPaths[i].path, index);
        break;
    case 1:
        snprintf_s(hdPath, maxLen, "%s", g_solPaths[i].path);
        break;
    case 2:
        snprintf_s(hdPath, maxLen, "%s/%u'/0'", g_solPaths[i].path, index);
        break;
    default:
        break;
    }
}

static void GetEthHdPath(char *hdPath, int index, uint32_t maxLen)
{
    uint8_t i = g_ethPathIndex[g_currentAccountIndex];
    if (g_multiPathCoinReceiveTileNow == RECEIVE_TILE_CHANGE_PATH) {
        i = g_selectType;
    }
    switch (i) {
    case 0:
        snprintf_s(hdPath, maxLen, "%s/0/%u", g_ethPaths[i].path, index);
        break;
    case 1:
        snprintf_s(hdPath, maxLen, "%s/%u'/0/0", g_ethPaths[i].path, index);
        break;
    case 2:
        snprintf_s(hdPath, maxLen, "%s/%u", g_ethPaths[i].path, index);
        break;
    default:
        break;
    }
}

static void GetEthRootPath(char *rootPath, int index, uint32_t maxLen)
{
    uint8_t i = g_ethPathIndex[g_currentAccountIndex];
    if (g_multiPathCoinReceiveTileNow == RECEIVE_TILE_CHANGE_PATH) {
        i = g_selectType;
    }
    switch (i) {
    case 0:
        snprintf_s(rootPath, maxLen, "%s", g_ethPaths[i].path);
        break;
    case 1:
        snprintf_s(rootPath, maxLen, "%s/%u'", g_ethPaths[i].path, index);
        break;
    case 2:
        snprintf_s(rootPath, maxLen, "%s", g_ethPaths[i].path);
        break;
    default:
        break;
    }
}

static char *GetSolXpub(int index)
{
    uint8_t i = g_solPathIndex[g_currentAccountIndex];
    if (g_multiPathCoinReceiveTileNow == RECEIVE_TILE_CHANGE_PATH) {
        i = g_selectType;
    }
    switch (i) {
    case 0:
        return GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_0 + index);
    case 1:
        return GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_ROOT);
    case 2:
        return GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_CHANGE_0 + index);
    default:
        break;
    }
    ASSERT(0);

    return "";
}

static char *GetEthXpub(int index)
{
    uint8_t i = g_ethPathIndex[g_currentAccountIndex];
    if (g_multiPathCoinReceiveTileNow == RECEIVE_TILE_CHANGE_PATH) {
        i = g_selectType;
    }
    switch (i) {
    case 0:
        return (char *)GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    case 1:
        return (char *)GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LIVE_0 + index);
    case 2:
        return (char *)GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY);
    default:
        break;
    }
    ASSERT(0);

    return "";
}

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item)
{

    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        ModelGetEthAddress(index, item);
        break;
    case HOME_WALLET_CARD_SOL:
        ModelGetSolAddress(index, item);
        break;
    default:
        break;
    }

}

static void ModelGetSolAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub = NULL, hdPath[BUFFER_SIZE_128] = {0};
    GetSolHdPath(hdPath, index, BUFFER_SIZE_128);
    xPub = GetSolXpub(index);
    ASSERT(xPub);
    SimpleResponse_c_char  *result = solana_get_address(xPub);
    if (result->error_code == 0) {
        item->index = index;
        strcpy_s(item->address, ADDRESS_MAX_LEN, result->data);
        strcpy_s(item->path, PATH_ITEM_MAX_LEN, hdPath);
    } else {
    }
    free_simple_response_c_char(result);
}

static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub, hdPath[BUFFER_SIZE_128], rootPath[BUFFER_SIZE_128];
    GetEthHdPath(hdPath, index, BUFFER_SIZE_128);
    GetEthRootPath(rootPath, index, BUFFER_SIZE_128);
    xPub = GetEthXpub(index);
    ASSERT(xPub);
    SimpleResponse_c_char  *result = eth_get_address(hdPath, xPub, rootPath);
    if (result->error_code == 0) {
        item->index = index;
        strcpy_s(item->address, ADDRESS_MAX_LEN, result->data);
        strcpy_s(item->path, PATH_ITEM_MAX_LEN, hdPath);
    }
    free_simple_response_c_char(result);
}

void GuiResetCurrentEthAddressIndex(uint8_t index)
{
    if (index > 2) {
        return;
    }

    g_selectIndex = 0;
    g_ethSelectIndex[index] = 0;
    g_solSelectIndex[index] = 0;
    g_ethPathIndex[index] = 0;
    g_solPathIndex[index] = 0;
}

void GuiResetAllEthAddressIndex(void)
{
    memset_s(g_ethSelectIndex, sizeof(g_ethSelectIndex), 0, sizeof(g_ethSelectIndex));
    memset_s(g_solSelectIndex, sizeof(g_solSelectIndex), 0, sizeof(g_solSelectIndex));
    memset_s(g_ethPathIndex, sizeof(g_ethPathIndex), 0, sizeof(g_ethPathIndex));
    memset_s(g_solPathIndex, sizeof(g_solPathIndex), 0, sizeof(g_solPathIndex));

}

static void SetCurrentSelectIndex(uint32_t selectIndex)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        g_ethSelectIndex[g_currentAccountIndex] = selectIndex;
        break;
    case HOME_WALLET_CARD_SOL:
        g_solSelectIndex[g_currentAccountIndex] = selectIndex;
        break;
    default:
        break;
    }
}

static uint32_t GetCurrentSelectIndex()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        return g_ethSelectIndex[g_currentAccountIndex];
    case HOME_WALLET_CARD_SOL:
        return g_solSelectIndex[g_currentAccountIndex];
    default:
        break;
    }
    return g_ethSelectIndex[g_currentAccountIndex];
}
#endif
