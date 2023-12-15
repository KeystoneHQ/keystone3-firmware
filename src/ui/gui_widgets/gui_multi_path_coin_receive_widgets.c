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

#define GENERAL_ADDRESS_INDEX_MAX               999999999
#define ETH_LEDGER_LIVE_ADDRESS_INDEX_MAX               9
#define SOL_BIP44_ADDRESS_INDEX_MAX                     9
#define SOL_BIP44_ROOT_ADDRESS_INDEX_MAX                0
#define SOL_BIP44_CHANGE_ADDRESS_INDEX_MAX              9
#define NEAR_BIP44_ADDRESS_INDEX_MAX                    0
#define NEAR_LEDGER_LIVE_ADDRESS_INDEX_MAX              9


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
    lv_obj_t *pathLabel;
    lv_obj_t *moreCont;
    lv_obj_t *addressButton;
    lv_obj_t *leftBtnImg;
    lv_obj_t *rightBtnImg;
    lv_obj_t *inputAddressLabel;
    lv_obj_t *overflowLabel;
    PathWidgetsItem_t changePathWidgets[3];
    SwitchAddressWidgetsItem_t switchAddressWidgets[5];
} MultiPathCoinReceiveWidgets_t;

typedef struct {
    GuiChainCoinType type;
    char tittle[32];
} TittleItem_t;

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

static void GetPathItemSubTitle(char* subTitle, int index);
static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item);
static void GetEthHdPath(char *hdPath, int index);
static void GetSolHdPath(char *hdPath, int index);
static void GetEthRootPath(char *rootPath, int index);
static char *GetEthXpub(int index);
static char *GetSolXpub(int index);
void AddressLongModeCut(char *out, const char *address);

static uint32_t GetPathIndex(void);
static void SetPathIndex(uint32_t index);

static int GetSOLMaxAddressIndex(void);
static int GetEthMaxAddressIndex(void);

static char* GetChangePathItemTitle(uint32_t i);
static void GetChangePathLabelHint(char* hint);

static void SetCurrentSelectIndex(uint32_t selectIndex);
static uint32_t GetCurrentSelectIndex();
static void GetHint(char *hint);
static void ModelGetSolAddress(uint32_t index, AddressDataItem_t *item);
static void ModelGetAddress(uint32_t index, AddressDataItem_t *item);
static void GetSolPathItemSubTitle(char* subTitle, int index);
static void GuiCreateNearWidget(lv_obj_t *parent);
static void ModelGetNearAddress(uint32_t index, AddressDataItem_t *item);

static MultiPathCoinReceiveWidgets_t g_multiPathCoinReceiveWidgets;
static EthereumReceiveTile g_multiPathCoinReceiveTileNow;

static const PathItem_t g_ethPaths[] = {
    {"BIP44 Standard",     "",     "m/44'/60'/0'"  },
    {"Ledger Live",         "",     "m/44'/60'"     },
    {"Ledger Legacy",       "",     "m/44'/60'/0'"  },
};
static const PathItem_t g_solPaths[] = {
    {"Account-based Path",                "",     "m/44'/501'"  },
    {"Single Account Path",     "",     "m/44'/501'"  },
    {"Sub-account Path",        "",     "m/44'/501'"  },
};
static const PathItem_t g_nearPaths[] = {
    {"Standard",     "",     "m/44'/397'/0'"     },
    {"Ledger Live",  "",     "m/44'/397'/0'/0'"  },
};
static lv_obj_t *g_addressLabel[2];
static lv_obj_t *g_goToAddressIcon;

static uint32_t g_showIndex;
static uint32_t g_selectIndex;
static uint8_t g_currentAccountIndex = 0;
static uint32_t g_ethSelectIndex[3] = {0};
static uint32_t g_solSelectIndex[3] = {0};
static uint32_t g_nearSelectIndex[3] = {0};
static uint32_t g_ethPathIndex[3] = {0};
static uint32_t g_solPathIndex[3] = {0};
static uint32_t g_nearPathIndex[3] = {0};
static PageWidget_t *g_pageWidget;
static HOME_WALLET_CARD_ENUM g_chainCard;
static lv_obj_t *g_derivationPathDescLabel = NULL;
static char * *g_derivationPathDescs = NULL;
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
    case HOME_WALLET_CARD_NEAR:
        g_derivationPathDescs = GetDerivationPathDescs(NEAR_DERIVATION_PATH_DESC);
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

    g_pageWidget = CreatePageWidget();
    g_multiPathCoinReceiveWidgets.cont = g_pageWidget->contentZone;
    g_multiPathCoinReceiveWidgets.tileView = lv_tileview_create(g_multiPathCoinReceiveWidgets.cont);
    lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_multiPathCoinReceiveWidgets.tileQrCode = lv_tileview_add_tile(g_multiPathCoinReceiveWidgets.tileView, RECEIVE_TILE_QRCODE, 0, LV_DIR_HOR);
    if (g_chainCard == HOME_WALLET_CARD_NEAR) {
        GuiCreateNearWidget(g_multiPathCoinReceiveWidgets.tileQrCode);
    } else {
        GuiCreateQrCodeWidget(g_multiPathCoinReceiveWidgets.tileQrCode);
    }
    g_multiPathCoinReceiveWidgets.tileSwitchAccount = lv_tileview_add_tile(g_multiPathCoinReceiveWidgets.tileView, RECEIVE_TILE_SWITCH_ACCOUNT, 0, LV_DIR_HOR);
    GuiCreateSwitchAddressWidget(g_multiPathCoinReceiveWidgets.tileSwitchAccount);
    GuiCreateSwitchAddressButtons(g_multiPathCoinReceiveWidgets.tileSwitchAccount);
    g_multiPathCoinReceiveWidgets.tileChangePath = lv_tileview_add_tile(g_multiPathCoinReceiveWidgets.tileView, RECEIVE_TILE_CHANGE_PATH, 0, LV_DIR_HOR);
    GuiCreateChangePathWidget(g_multiPathCoinReceiveWidgets.tileChangePath);
    lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.tileView, LV_OBJ_FLAG_SCROLLABLE);
}

void GuiMultiPathCoinReceiveDeInit(void)
{
    SetCurrentSelectIndex(g_selectIndex);
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
    switch (g_multiPathCoinReceiveTileNow) {
    case RECEIVE_TILE_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        switch (g_chainCard) {
        case HOME_WALLET_CARD_ETH:
            SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_ETH, "Receive ETH");
            break;
        case HOME_WALLET_CARD_SOL:
            SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_SOL, "Receive SOL");
            break;
        case HOME_WALLET_CARD_NEAR:
            SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_NEAR, "Receive NEAR");
            break;
        default:
            break;
        }
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, MoreHandler, NULL);
        RefreshQrCode();
        break;
    case RECEIVE_TILE_SWITCH_ACCOUNT:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("switch_account"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
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
        break;
    case RECEIVE_TILE_CHANGE_PATH:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("derivation_path_change"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
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

    g_multiPathCoinReceiveWidgets.moreCont = GuiCreateHintBox(parent, 480, 228, true);
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
    label = GuiCreateLabelWithFont(btn, _("derivation_path_change"), &openSans_24);
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
    label = GuiCreateLabelWithFont(btn, _("Tutorial"), &openSans_24);
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

static void GuiFirstReceive(lv_obj_t *parent) {
    const char* coin = GetCoinCardByIndex(g_chainCard)->coin;
    if (!GetFirstReceive(coin)) {
        g_multiPathCoinReceiveWidgets.attentionCont = GuiCreateHintBox(parent, 480, 386, false);
        lv_obj_t *tempObj = GuiCreateImg(g_multiPathCoinReceiveWidgets.attentionCont, &imgInformation);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 462);
        tempObj = GuiCreateLittleTitleLabel(g_multiPathCoinReceiveWidgets.attentionCont, _("Attention"));
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 558);
        char hint[256];
        GetHint(hint);
        tempObj = GuiCreateLabelWithFont(g_multiPathCoinReceiveWidgets.attentionCont, hint, &openSans_20);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 610);
        tempObj = GuiCreateBtn(g_multiPathCoinReceiveWidgets.attentionCont, _("got_it"));
        lv_obj_set_size(tempObj, 122, 66);
        lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
        SetFirstReceive(coin, true);
    }
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

    GuiFirstReceive(parent);
}

static void GuiCreateNearWidget(lv_obj_t *parent)
{
    g_multiPathCoinReceiveWidgets.qrCodeCont = GuiCreateContainerWithParent(parent, 408, 336);
    uint16_t yOffset = 36;
    lv_obj_t *line;
    static lv_point_t points[2] =  {{0, 0}, {336, 0}};
    lv_obj_align(g_multiPathCoinReceiveWidgets.qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_multiPathCoinReceiveWidgets.qrCodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_multiPathCoinReceiveWidgets.qrCodeCont, 24, LV_PART_MAIN);

    g_multiPathCoinReceiveWidgets.addressCountLabel = GuiCreateLabel(g_multiPathCoinReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_multiPathCoinReceiveWidgets.addressCountLabel, LV_ALIGN_DEFAULT, 36, yOffset);

    yOffset += 46;
    line = GuiCreateLine(g_multiPathCoinReceiveWidgets.qrCodeCont, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 36, yOffset);

    yOffset += 18;
    lv_obj_t *tempObj = GuiCreateNoticeLabel(g_multiPathCoinReceiveWidgets.qrCodeCont, _("public_key"));
    lv_obj_align(tempObj, LV_ALIGN_DEFAULT, 36, yOffset);

    yOffset += 38;
    g_multiPathCoinReceiveWidgets.addressLabel = GuiCreateNoticeLabel(g_multiPathCoinReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_multiPathCoinReceiveWidgets.addressLabel, LV_ALIGN_DEFAULT, 36, yOffset);
    lv_obj_set_width(g_multiPathCoinReceiveWidgets.addressLabel, 336);

    yOffset += 106;
    tempObj = GuiCreateNoticeLabel(g_multiPathCoinReceiveWidgets.qrCodeCont, _("path"));
    lv_obj_align(tempObj, LV_ALIGN_DEFAULT, 36, yOffset);

    yOffset += 38;
    g_multiPathCoinReceiveWidgets.pathLabel = GuiCreateNoticeLabel(g_multiPathCoinReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_multiPathCoinReceiveWidgets.pathLabel, LV_ALIGN_DEFAULT, 36, yOffset);

    g_multiPathCoinReceiveWidgets.addressButton = lv_btn_create(g_multiPathCoinReceiveWidgets.qrCodeCont);
    lv_obj_set_size(g_multiPathCoinReceiveWidgets.addressButton, 336, 36);
    lv_obj_align(g_multiPathCoinReceiveWidgets.addressButton, LV_ALIGN_TOP_LEFT, 36, 26);
    lv_obj_set_style_bg_opa(g_multiPathCoinReceiveWidgets.addressButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_multiPathCoinReceiveWidgets.addressButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_multiPathCoinReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_multiPathCoinReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_add_flag(g_multiPathCoinReceiveWidgets.addressButton, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g_multiPathCoinReceiveWidgets.addressButton, OpenSwitchAddressHandler, LV_EVENT_CLICKED, NULL);
    g_goToAddressIcon = GuiCreateImg(g_multiPathCoinReceiveWidgets.addressButton, &imgArrowRight);
    lv_obj_align(g_goToAddressIcon, LV_ALIGN_RIGHT_MID, 20, 0);

    GuiFirstReceive(parent);
}

static void GetHint(char *hint)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        strcpy(hint, _("receive_eth_alert_desc"));
        break;
    case HOME_WALLET_CARD_SOL:
        sprintf(hint, _("receive_coin_hint_fmt"), "SOL");
        break;
    case HOME_WALLET_CARD_NEAR:
        sprintf(hint, _("receive_coin_hint_fmt"), "NEAR");
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
        g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel = GuiCreateLabelWithFont(cont, "", &openSans_24);
        lv_obj_align(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
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
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    img = GuiCreateImg(btn, &imgArrowRight);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, RightBtnHandler, LV_EVENT_CLICKED, NULL);
    g_multiPathCoinReceiveWidgets.rightBtnImg = img;
}

static void GetChangePathLabelHint(char* hint)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        sprintf(hint, _("derivation_path_select_eth"));
        return;
    case HOME_WALLET_CARD_SOL:
        sprintf(hint, _("derivation_path_select_sol"));
        return;
    case HOME_WALLET_CARD_NEAR:
        sprintf(hint, _("derivation_path_select_near"));
        return;
    default:
        break;
    }
}

static char* GetChangePathItemTitle(uint32_t i)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        return (char *)g_ethPaths[i].title;
    case HOME_WALLET_CARD_SOL:
        return (char *)g_solPaths[i].title;
    case HOME_WALLET_CARD_NEAR:
        return (char *)g_nearPaths[i].title;
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
    label = GuiCreateNoticeLabel(egCont, g_derivationPathDescs[GetPathIndex()]);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    lv_obj_update_layout(label);
    egContHeight += lv_obj_get_height(label);
    g_derivationPathDescLabel = label;
    prevLabel = label;

    int gap = 4;
    if (strlen(g_derivationPathDescs[GetPathIndex()]) == 0) {
        egContHeight -= lv_obj_get_height(label);
        lv_obj_set_height(label, 0);
        gap = 0;
    }

    char *desc = _("derivation_path_address_eg");
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

    if (!((g_chainCard == HOME_WALLET_CARD_SOL && g_solPathIndex[g_currentAccountIndex] == 1) || (g_chainCard == HOME_WALLET_CARD_NEAR && g_nearPathIndex[g_currentAccountIndex] == 0)))
    {
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
    char string[64];
    char lableText[128] = {0};
    GetChangePathLabelHint(lableText);
    lv_obj_t *labelHint = GuiCreateIllustrateLabel(parent, lableText);
    lv_obj_set_style_text_opa(labelHint, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 36, 0);

    cont = GuiCreateContainerWithParent(parent, 408, 308);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);

    int8_t len = 3;
    if (g_chainCard == HOME_WALLET_CARD_NEAR) {
        len = 2;
        lv_obj_set_height(cont, 205);
    }
    for (uint32_t i = 0; i < len; i++) {
        label = GuiCreateLabelWithFont(cont, GetChangePathItemTitle(i), &openSans_24);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
        GetPathItemSubTitle(string, i);
        label = GuiCreateLabelWithFontAndTextColor(cont, string, g_defIllustrateFont, 0x919191);
        lv_label_set_recolor(label, true);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i < len - 1){
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
    lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[GetPathIndex()].checkedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[GetPathIndex()].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *egCont = GuiCreateContainerWithParent(parent, 408, 186);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);
    g_egCont = egCont;
    ShowEgAddressCont(g_egCont);
}

static void RefreshQrCode(void)
{
    AddressDataItem_t addressDataItem;
    ModelGetAddress(g_selectIndex, &addressDataItem);
    if (g_chainCard != HOME_WALLET_CARD_NEAR) {
        lv_qrcode_update(g_multiPathCoinReceiveWidgets.qrCode, addressDataItem.address, strlen(addressDataItem.address));
        lv_obj_t *fullscreen_qrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
        if (fullscreen_qrcode) {
            lv_qrcode_update(fullscreen_qrcode, addressDataItem.address, strlen(addressDataItem.address));
        }
    }
    lv_label_set_text(g_multiPathCoinReceiveWidgets.addressLabel, addressDataItem.address);
    lv_label_set_text_fmt(g_multiPathCoinReceiveWidgets.addressCountLabel, "Account-%u", (addressDataItem.index + 1));
    if (g_chainCard == HOME_WALLET_CARD_NEAR) {
        lv_label_set_text(g_multiPathCoinReceiveWidgets.pathLabel, addressDataItem.path);
        if (GetPathIndex() == 0) {
            lv_obj_add_flag(g_multiPathCoinReceiveWidgets.addressButton, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.addressButton, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_multiPathCoinReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "Account-%u", (addressDataItem.index + 1));
        AddressLongModeCut(string, addressDataItem.address);
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

static void RefreshDefaultAddress(void)
{
    char string[128];

    AddressDataItem_t addressDataItem;

    ModelGetAddress(0, &addressDataItem);
    AddressLongModeCut(string, addressDataItem.address);
    lv_label_set_text(g_addressLabel[0], string);

    if (GetMaxAddressIndex() != 0) {
        ModelGetAddress(1, &addressDataItem);
        AddressLongModeCut(string, addressDataItem.address);
        lv_label_set_text(g_addressLabel[1], string);
        lv_obj_clear_flag(g_goToAddressIcon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_goToAddressIcon, LV_OBJ_FLAG_HIDDEN);
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

static int GetNearMaxAddressIndex(void)
{
    switch (g_nearPathIndex[GetCurrentAccountIndex()]) {
    case 0:
        return NEAR_BIP44_ADDRESS_INDEX_MAX;
    case 1:
        return NEAR_LEDGER_LIVE_ADDRESS_INDEX_MAX;
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
    case HOME_WALLET_CARD_NEAR:
        return GetNearMaxAddressIndex();
    default:
        break;
    }

    return -1;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(g_multiPathCoinReceiveWidgets.attentionCont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void MoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_multiPathCoinReceiveWidgets.moreCont == NULL) {
            GuiCreateMoreWidgets(g_multiPathCoinReceiveWidgets.tileQrCode);
        } else {
            lv_obj_del(g_multiPathCoinReceiveWidgets.moreCont);
            g_multiPathCoinReceiveWidgets.moreCont = NULL;
        }
    }
}

static void ChangePathHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_multiPathCoinReceiveWidgets.moreCont != NULL) {
            lv_obj_del(g_multiPathCoinReceiveWidgets.moreCont);
            g_multiPathCoinReceiveWidgets.moreCont = NULL;
        }
        GuiEthereumReceiveGotoTile(RECEIVE_TILE_CHANGE_PATH);
    }
}

static TUTORIAL_LIST_INDEX_ENUM GetTutorialIndex()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        return TUTORIAL_ETH_RECEIVE;
    case HOME_WALLET_CARD_SOL:
        return TUTORIAL_SOL_RECEIVE;
    case HOME_WALLET_CARD_NEAR:
        return TUTORIAL_NEAR_RECEIVE;
    default:
        break;
    }

    return TUTORIAL_LIST_INDEX_BUTT;
}

static void TutorialHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_multiPathCoinReceiveWidgets.moreCont);

        TUTORIAL_LIST_INDEX_ENUM index = GetTutorialIndex();
        GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
    }
}


static void LeftBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex >= 5) {
            g_showIndex -= 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
    }
}


static void RightBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex < GetMaxAddressIndex() - 5) {
            g_showIndex += 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_multiPathCoinReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
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
    case HOME_WALLET_CARD_NEAR:
        return g_nearPathIndex[g_currentAccountIndex];
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
    case HOME_WALLET_CARD_NEAR:
        g_nearPathIndex[g_currentAccountIndex] = index;
        break;
    default:
        break;
    }
}
static void ChangePathCheckHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkBox;
    int8_t len = (g_chainCard == HOME_WALLET_CARD_NEAR ? 2 : 3);

    if (code == LV_EVENT_CLICKED) {
        checkBox = lv_event_get_target(e);
        for (uint32_t i = 0; i < len; i++) {
            if (checkBox == g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox) {
                lv_obj_add_state(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                if (GetPathIndex() != i) {
                    SetPathIndex(i);
                    g_selectIndex = 0;
                    g_showIndex = 0;
                    ShowEgAddressCont(g_egCont);
                }
            } else {
                lv_obj_clear_state(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_add_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_multiPathCoinReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
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
    }
}

static void OpenSwitchAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (GetMaxAddressIndex() == 0) {
            return;
        }
        GuiEthereumReceiveGotoTile(RECEIVE_TILE_SWITCH_ACCOUNT);
        RefreshSwitchAccount();
    }
}



static void GetEthPathItemSubTittle(char* subTitle, int index)
{
    switch (index) {
    case 0:
        sprintf(subTitle, "m/44'/60'/0'/0/#F5870A X#");
        break;
    case 1:
        sprintf(subTitle, "m/44'/60'/#F5870A X#'/0/0");
        break;
    case 2:
        sprintf(subTitle, "m/44'/60'/0'/#F5870A X#");
        break;
    default:
        break;
    }
}

static void GetSolPathItemSubTitle(char* subTitle, int index)
{
    switch (index) {
    case 0:
        sprintf(subTitle, "m/44'/501'/#F5870A X#'");
        break;
    case 1:
        sprintf(subTitle, "m/44'/501'");
        break;
    case 2:
        sprintf(subTitle, "m/44'/501'/#F5870A X#'/0'");
        break;
    default:
        break;
    }
}

static void GetNearPathItemSubTitle(char* subTitle, int index)
{
    switch (index) {
    case 0:
        sprintf(subTitle, "m/44'/397'/0'");
        break;
    case 1:
        sprintf(subTitle, "m/44'/397'/0'/0'/#F5870A X#'");
        break;
    default:
        break;
    }
}

static void GetPathItemSubTitle(char* subTitle, int index)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_ETH:
        GetEthPathItemSubTittle(subTitle, index);
        break;
    case HOME_WALLET_CARD_SOL:
        GetSolPathItemSubTitle(subTitle, index);
        break;
    case HOME_WALLET_CARD_NEAR:
        GetNearPathItemSubTitle(subTitle, index);
        break;
    default:
        break;
    }
}

void AddressLongModeCut(char *out, const char *address)
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

static void GetNearHdPath(char *hdPath, int index)
{
    switch (g_nearPathIndex[g_currentAccountIndex]) {
    case 0:
        sprintf(hdPath, "%s", g_nearPaths[g_nearPathIndex[g_currentAccountIndex]].path);
        break;
    case 1:
        sprintf(hdPath, "%s/%u'", g_nearPaths[g_nearPathIndex[g_currentAccountIndex]].path, index);
        break;
    default:
        break;
    }
}

static void GetSolHdPath(char *hdPath, int index)
{
    switch (g_solPathIndex[g_currentAccountIndex]) {
    case 0:
        sprintf(hdPath, "%s/%u'", g_solPaths[g_solPathIndex[g_currentAccountIndex]].path, index);
        break;
    case 1:
        sprintf(hdPath, "%s", g_solPaths[g_solPathIndex[g_currentAccountIndex]].path);
        break;
    case 2:
        sprintf(hdPath, "%s/%u'/0'", g_solPaths[g_solPathIndex[g_currentAccountIndex]].path, index);
        break;
    default:
        break;
    }
}

static void GetEthHdPath(char *hdPath, int index)
{
    switch (g_ethPathIndex[g_currentAccountIndex]) {
    case 0:
        sprintf(hdPath, "%s/0/%u", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path, index);
        break;
    case 1:
        sprintf(hdPath, "%s/%u'/0/0", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path, index);
        break;
    case 2:
        sprintf(hdPath, "%s/%u", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path, index);
        break;
    default:
        break;
    }
}

static void GetEthRootPath(char *rootPath, int index)
{
    switch (g_ethPathIndex[g_currentAccountIndex]) {
    case 0:
        sprintf(rootPath, "%s", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path);
        break;
    case 1:
        sprintf(rootPath, "%s/%u'", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path, index);
        break;
    case 2:
        sprintf(rootPath, "%s", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path);
        break;
    default:
        break;
    }
}

static char *GetSolXpub(int index)
{
    switch (g_solPathIndex[g_currentAccountIndex]) {
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

static char *GetNearXpub(int index)
{
    switch (g_nearPathIndex[g_currentAccountIndex]) {
    case 0:
        return (char *)GetCurrentAccountPublicKey(XPUB_TYPE_NEAR_BIP44_STANDARD_0);
    case 1:
        return (char *)GetCurrentAccountPublicKey(XPUB_TYPE_NEAR_LEDGER_LIVE_0 + index);
    default:
        break;
    }
    ASSERT(0);

    return "";
}

static char *GetEthXpub(int index)
{
    switch (g_ethPathIndex[g_currentAccountIndex]) {
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
    case HOME_WALLET_CARD_NEAR:
        ModelGetNearAddress(index, item);
        break;
    default:
        break;
    }

}

#ifdef COMPILE_SIMULATOR

static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item)
{
    char hdPath[128];
    //sprintf(hdPath, "m/44'/0'/0'/0/%u", index);
    sprintf(hdPath, "%s/0/%u", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path, index);
    printf("hdPath=%s\r\n", hdPath);
    item->index = index;
    sprintf(item->address, "tb1qkcp7vdhczgk5eh59d2l0dxvmpzhx%010u", index);
    strcpy(item->path, hdPath);
}

static void ModelGetSolAddress(uint32_t index, AddressDataItem_t *item)
{
    char hdPath[128];
    //sprintf(hdPath, "m/44'/0'/0'/0/%u", index);
    sprintf(hdPath, "%s/0/%u", g_ethPaths[g_ethPathIndex[g_currentAccountIndex]].path, index);
    printf("hdPath=%s\r\n", hdPath);
    item->index = index;
    sprintf(item->address, "tb1qkcp7vdhczgk5eh59d2l0dxvmpzhx%010u", index);
    strcpy(item->path, hdPath);
}

static void ModelGetNearAddress(uint32_t index, AddressDataItem_t *item)
{
    char hdPath[128];
    GetNearHdPath(hdPath, index);
    printf("ModelGetNearAddress hdPath=%s\r\n", hdPath);
    item->index = index;
    sprintf(item->address, "tb1qkcp7vdhczgk5eh59d2l0dxvmpzhxasjdhakjsdhjasduwbdshwuhtqjhbfwiuehf%u", index);
    strcpy(item->path, hdPath);
}

#else


static void ModelGetNearAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub = NULL, hdPath[128] = {0};
    GetNearHdPath(hdPath, index);
    xPub = GetNearXpub(index);
    ASSERT(xPub);
    item->index = index;
    strcpy(item->address, xPub);
    strcpy(item->path, hdPath);
}

static void ModelGetSolAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub = NULL, hdPath[128] = {0};
    GetSolHdPath(hdPath, index);
    xPub = GetSolXpub(index);
    ASSERT(xPub);
    SimpleResponse_c_char  *result = solana_get_address(xPub);
    if (result->error_code == 0) {
        item->index = index;
        strcpy(item->address, result->data);
        strcpy(item->path, hdPath);
    } else {
    }
    free_simple_response_c_char(result);
}


static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub, hdPath[128], rootPath[128];
    GetEthHdPath(hdPath, index);
    GetEthRootPath(rootPath, index);
    xPub = GetEthXpub(index);
    ASSERT(xPub);
    SimpleResponse_c_char  *result = eth_get_address(hdPath, xPub, rootPath);
    if (result->error_code == 0) {
        item->index = index;
        strcpy(item->address, result->data);
        strcpy(item->path, hdPath);
    } else {
    }
    free_simple_response_c_char(result);
}

#endif

void GuiResetCurrentEthAddressIndex(uint8_t index)
{
    if (index > 2)
    {
        return;
    }
    
    g_selectIndex = 0;
    g_ethSelectIndex[index] = 0;
    g_solSelectIndex[index] = 0;
    g_nearSelectIndex[index] = 0;
    g_ethPathIndex[index] = 0;
    g_solPathIndex[index] = 0;
    g_nearPathIndex[index] = 0;
}

void GuiResetAllEthAddressIndex(void)
{
    memset(g_ethSelectIndex, 0, sizeof(g_ethSelectIndex));
    memset(g_solSelectIndex, 0, sizeof(g_solSelectIndex));
    memset(g_nearSelectIndex, 0, sizeof(g_nearSelectIndex));
    memset(g_ethPathIndex, 0, sizeof(g_ethPathIndex));
    memset(g_solPathIndex, 0, sizeof(g_solPathIndex));
    memset(g_nearPathIndex, 0, sizeof(g_nearPathIndex));
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
    case HOME_WALLET_CARD_NEAR:
        g_nearSelectIndex[g_currentAccountIndex] = selectIndex;
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
    case HOME_WALLET_CARD_NEAR:
        return g_nearSelectIndex[g_currentAccountIndex];
    default:
        break;
    }
    return g_ethSelectIndex[g_currentAccountIndex];
}
