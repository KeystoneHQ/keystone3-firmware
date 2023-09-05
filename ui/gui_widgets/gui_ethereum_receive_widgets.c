#include "gui_ethereum_receive_widgets.h"
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

#define GENERAL_ADDRESS_INDEX_MAX               999999999
#define LEDGER_LIVE_ADDRESS_INDEX_MAX               9

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
    lv_obj_t *inputAddressLabel;
    lv_obj_t *overflowLabel;
    PathWidgetsItem_t changePathWidgets[3];
    SwitchAddressWidgetsItem_t switchAddressWidgets[5];
} EthereumReceiveWidgets_t;

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
static void GuiCreateSwitchAccountWidget(lv_obj_t *parent);
static void GuiCreateSwitchAccountButtons(lv_obj_t *parent);
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
static void SelectAddressHandler(lv_event_t *e);

static void GetPathItemSubTittle(char* subTittle, int index);
static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item);
static void GetHdPath(char *hdPath, int index);
static void GetRootPath(char *rootPath, int index);
static char *GetXpub(int index);
void AddressLongModeCut(char *out, const char *address);

static EthereumReceiveWidgets_t g_ethereumReceiveWidgets;
static EthereumReceiveTile g_EthereumReceiveTileNow;
static const PathItem_t g_paths[] = {
    {"BIP 44 Standard",     "",     "m/44'/60'/0'"  },
    {"Ledger Live",         "",     "m/44'/60'"     },
    {"Ledger Legacy",       "",     "m/44'/60'/0'"  },
};
static lv_obj_t *g_addressLabel[2];

//to do: stored.
static uint32_t g_showIndex;
static uint32_t g_selectIndex[3] = {0};
static uint32_t g_pathIndex[3] = {0};

void GuiEthereumReceiveInit(void)
{
    g_ethereumReceiveWidgets.cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_align(g_ethereumReceiveWidgets.cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);
    g_ethereumReceiveWidgets.tileView = lv_tileview_create(g_ethereumReceiveWidgets.cont);
    lv_obj_set_style_bg_opa(g_ethereumReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_ethereumReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_ethereumReceiveWidgets.tileQrCode = lv_tileview_add_tile(g_ethereumReceiveWidgets.tileView, RECEIVE_TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_ethereumReceiveWidgets.tileQrCode);
    g_ethereumReceiveWidgets.tileSwitchAccount = lv_tileview_add_tile(g_ethereumReceiveWidgets.tileView, RECEIVE_TILE_SWITCH_ACCOUNT, 0, LV_DIR_HOR);
    GuiCreateSwitchAccountWidget(g_ethereumReceiveWidgets.tileSwitchAccount);
    GuiCreateSwitchAccountButtons(g_ethereumReceiveWidgets.tileSwitchAccount);
    g_ethereumReceiveWidgets.tileChangePath = lv_tileview_add_tile(g_ethereumReceiveWidgets.tileView, RECEIVE_TILE_CHANGE_PATH, 0, LV_DIR_HOR);
    GuiCreateChangePathWidget(g_ethereumReceiveWidgets.tileChangePath);
    lv_obj_clear_flag(g_ethereumReceiveWidgets.tileView, LV_OBJ_FLAG_SCROLLABLE);
}

void GuiEthereumReceiveDeInit(void)
{
    GUI_DEL_OBJ(g_ethereumReceiveWidgets.moreCont)
    GUI_DEL_OBJ(g_ethereumReceiveWidgets.attentionCont)
    GUI_DEL_OBJ(g_ethereumReceiveWidgets.cont)

    CLEAR_OBJECT(g_ethereumReceiveWidgets);
    g_EthereumReceiveTileNow = 0;
    GuiFullscreenModeCleanUp();
}

void GuiEthereumReceiveRefresh(void)
{
    switch (g_EthereumReceiveTileNow) {
    case RECEIVE_TILE_QRCODE:
        GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        GuiNvsSetCoinWallet(CHAIN_ETH, "Receive ETH");
        GuiNvsBarSetRightCb(NVS_BAR_MORE_INFO, MoreHandler, NULL);
        RefreshQrCode();
        break;
    case RECEIVE_TILE_SWITCH_ACCOUNT:
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "Switch Account");
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        g_showIndex = g_selectIndex[GetCurrentAccountIndex()] / 5 * 5;
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        } else if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        } else {
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        }
        break;
    case RECEIVE_TILE_CHANGE_PATH:
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "Change Derivation Path");
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    default:
        break;
    }
}

void GuiEthereumReceivePrevTile(void)
{
    GuiEthereumReceiveGotoTile(RECEIVE_TILE_QRCODE);
}

static void GuiCreateMoreWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *btn, *img, *label;

    g_ethereumReceiveWidgets.moreCont = GuiCreateHintBox(parent, 480, 228, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_ethereumReceiveWidgets.moreCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_ethereumReceiveWidgets.moreCont);
    cont = g_ethereumReceiveWidgets.moreCont;

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
    label = GuiCreateLabelWithFont(btn, "Change Derivation Path", &openSans_24);
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
    label = GuiCreateLabelWithFont(btn, "Tutorial", &openSans_24);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);
}

static void GuiEthereumReceiveGotoTile(EthereumReceiveTile tile)
{
    g_EthereumReceiveTileNow = tile;
    GuiEthereumReceiveRefresh();
    lv_obj_set_tile_id(g_ethereumReceiveWidgets.tileView, g_EthereumReceiveTileNow, 0, LV_ANIM_OFF);
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

    g_ethereumReceiveWidgets.qrCodeCont = GuiCreateContainerWithParent(parent, 408, 524);
    lv_obj_align(g_ethereumReceiveWidgets.qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_ethereumReceiveWidgets.qrCodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_ethereumReceiveWidgets.qrCodeCont, 24, LV_PART_MAIN);

    yOffset += 36;
    g_ethereumReceiveWidgets.qrCode = CreateEthereumReceiveQRCode(g_ethereumReceiveWidgets.qrCodeCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(CreateEthereumReceiveQRCode, 420, 420);
    lv_obj_align(g_ethereumReceiveWidgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 16;
    g_ethereumReceiveWidgets.addressLabel = GuiCreateNoticeLabel(g_ethereumReceiveWidgets.qrCodeCont, "");
    lv_obj_set_width(g_ethereumReceiveWidgets.addressLabel, 336);
    lv_obj_align(g_ethereumReceiveWidgets.addressLabel, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 60;

    yOffset += 16;
    g_ethereumReceiveWidgets.addressCountLabel = GuiCreateIllustrateLabel(g_ethereumReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_ethereumReceiveWidgets.addressCountLabel, LV_ALIGN_TOP_LEFT, 36, yOffset);

    g_ethereumReceiveWidgets.addressButton = lv_btn_create(g_ethereumReceiveWidgets.qrCodeCont);
    lv_obj_set_size(g_ethereumReceiveWidgets.addressButton, 336, 36);
    lv_obj_align(g_ethereumReceiveWidgets.addressButton, LV_ALIGN_TOP_MID, 0, 464);
    lv_obj_set_style_bg_opa(g_ethereumReceiveWidgets.addressButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_ethereumReceiveWidgets.addressButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_ethereumReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_ethereumReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(g_ethereumReceiveWidgets.addressButton, SelectAddressHandler, LV_EVENT_CLICKED, NULL);
    tempObj = GuiCreateImg(g_ethereumReceiveWidgets.addressButton, &imgArrowRight);
    lv_obj_set_style_img_opa(tempObj, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(tempObj, LV_ALIGN_CENTER, 150, 0);

    if (!GetFirstReceive("ETH")) {
        g_ethereumReceiveWidgets.attentionCont = GuiCreateHintBox(parent, 480, 386, false);
        tempObj = GuiCreateImg(g_ethereumReceiveWidgets.attentionCont, &imgInformation);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 462);
        tempObj = GuiCreateLittleTitleLabel(g_ethereumReceiveWidgets.attentionCont, "Attention");
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 558);
        tempObj = GuiCreateLabelWithFont(g_ethereumReceiveWidgets.attentionCont, "This address is only for ETH and EVM ERC-20 tokens, other digital assets sent to this address will be lost.", &openSans_20);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 610);
        tempObj = GuiCreateBtn(g_ethereumReceiveWidgets.attentionCont, "Got It");
        lv_obj_set_size(tempObj, 122, 66);
        lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
        SetFirstReceive("ETH", true);
    }
}

static void GuiCreateSwitchAccountWidget(lv_obj_t *parent)
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
        g_ethereumReceiveWidgets.switchAddressWidgets[i].addressCountLabel = GuiCreateLabelWithFont(cont, "", &openSans_24);
        lv_obj_align(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
        g_ethereumReceiveWidgets.switchAddressWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "");
        lv_obj_align(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, SwitchAddressHandler, LV_EVENT_CLICKED, NULL);

        g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg = GuiCreateImg(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg = GuiCreateImg(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

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
    g_ethereumReceiveWidgets.leftBtnImg = img;

    btn = GuiCreateBtn(parent, "");
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    img = GuiCreateImg(btn, &imgArrowRight);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, RightBtnHandler, LV_EVENT_CLICKED, NULL);
    g_ethereumReceiveWidgets.rightBtnImg = img;
}

static void GuiCreateChangePathWidget(lv_obj_t *parent)
{
    lv_obj_t *cont, *line, *label;
    static lv_point_t points[2] =  {{0, 0}, {360, 0}};
    char string[64];

    lv_obj_t *labelHint = GuiCreateIllustrateLabel(parent, _("Select the derivation path you’d like to use for Ethereum"));
    lv_obj_set_style_text_opa(labelHint, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 36, 0);

    cont = GuiCreateContainerWithParent(parent, 408, 308);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);

    for (uint32_t i = 0; i < 3; i++) {
        label = GuiCreateLabelWithFont(cont, g_paths[i].title, &openSans_24);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
        GetPathItemSubTittle(string, i);
        label = GuiCreateLabelWithFontAndTextColor(cont, string, g_defIllustrateFont, 0x919191);
        lv_label_set_recolor(label, true);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        line = GuiCreateLine(cont, points, 2);
        lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * (i + 1));

        g_ethereumReceiveWidgets.changePathWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, ChangePathCheckHandler, LV_EVENT_CLICKED, NULL);

        g_ethereumReceiveWidgets.changePathWidgets[i].checkedImg = GuiCreateImg(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_ethereumReceiveWidgets.changePathWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_ethereumReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_ethereumReceiveWidgets.changePathWidgets[i].uncheckedImg = GuiCreateImg(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_ethereumReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_ethereumReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_clear_flag(g_ethereumReceiveWidgets.changePathWidgets[g_pathIndex[GetCurrentAccountIndex()]].checkedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_ethereumReceiveWidgets.changePathWidgets[g_pathIndex[GetCurrentAccountIndex()]].uncheckedImg, LV_OBJ_FLAG_HIDDEN);


    cont = GuiCreateContainerWithParent(parent, 408, 186);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 416);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);

    labelHint = GuiCreateIllustrateLabel(cont, "Recommend. Most commonly used in many software wallets.");
    lv_obj_set_size(labelHint, 360, 60);
    lv_obj_set_style_text_opa(labelHint, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 24, 12);

    labelHint = GuiCreateIllustrateLabel(cont, "Addresses eg.");
    lv_obj_set_size(labelHint, 129, 30);
    lv_obj_set_style_text_opa(labelHint, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 24, 76);

    labelHint = GuiCreateIllustrateLabel(cont, "0");
    lv_obj_set_style_text_opa(labelHint, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 24, 110);

    lv_obj_t *defaultLable1 = GuiCreateIllustrateLabel(cont, "");
    lv_obj_set_style_text_opa(defaultLable1, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(defaultLable1, LV_ALIGN_TOP_LEFT, 48, 110);

    labelHint = GuiCreateIllustrateLabel(cont, "1");
    lv_obj_set_style_text_opa(labelHint, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 24, 144);


    lv_obj_t *defaultLable2 = GuiCreateIllustrateLabel(cont, "");
    lv_obj_set_style_text_opa(defaultLable2, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(defaultLable2, LV_ALIGN_TOP_LEFT, 48, 144);

    g_addressLabel[0] = defaultLable1;
    g_addressLabel[1] = defaultLable2;
    RefreshDefaultAddress();
}

static void RefreshQrCode(void)
{
    AddressDataItem_t addressDataItem;

    ModelGetEthAddress(g_selectIndex[GetCurrentAccountIndex()], &addressDataItem);
    lv_qrcode_update(g_ethereumReceiveWidgets.qrCode, addressDataItem.address, strlen(addressDataItem.address));
    lv_obj_t *fullscreen_qrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullscreen_qrcode) {
        lv_qrcode_update(fullscreen_qrcode, addressDataItem.address, strlen(addressDataItem.address));
    }
    lv_label_set_text(g_ethereumReceiveWidgets.addressLabel, addressDataItem.address);
    lv_label_set_text_fmt(g_ethereumReceiveWidgets.addressCountLabel, "Account-%u", (addressDataItem.index + 1));
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetEthAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "Account-%u", (addressDataItem.index + 1));
        AddressLongModeCut(string, addressDataItem.address);
        lv_label_set_text(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == g_selectIndex[GetCurrentAccountIndex()]) {
            lv_obj_add_state(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
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

    ModelGetEthAddress(0, &addressDataItem);
    AddressLongModeCut(string, addressDataItem.address);
    lv_label_set_text(g_addressLabel[0], string);

    ModelGetEthAddress(1, &addressDataItem);
    AddressLongModeCut(string, addressDataItem.address);
    lv_label_set_text(g_addressLabel[1], string);
}

static int GetMaxAddressIndex(void)
{
    switch (g_pathIndex[GetCurrentAccountIndex()]) {
    case 0:
    case 2:
        return GENERAL_ADDRESS_INDEX_MAX;
    case 1:
        return LEDGER_LIVE_ADDRESS_INDEX_MAX;
    default:
        break;
    }
    return GENERAL_ADDRESS_INDEX_MAX;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(g_ethereumReceiveWidgets.attentionCont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void MoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_ethereumReceiveWidgets.moreCont == NULL) {
            GuiCreateMoreWidgets(g_ethereumReceiveWidgets.tileQrCode);
        } else {
            lv_obj_del(g_ethereumReceiveWidgets.moreCont);
            g_ethereumReceiveWidgets.moreCont = NULL;
        }
    }
}

static void ChangePathHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_ethereumReceiveWidgets.moreCont != NULL) {
            lv_obj_del(g_ethereumReceiveWidgets.moreCont);
            g_ethereumReceiveWidgets.moreCont = NULL;
        }
        GuiEthereumReceiveGotoTile(RECEIVE_TILE_CHANGE_PATH);
    }
}


static void TutorialHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_ethereumReceiveWidgets.moreCont);

        TUTORIAL_LIST_INDEX_ENUM index = TUTORIAL_ETH_RECEIVE;
        GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
    }
}


static void LeftBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex >= 5) {
            g_showIndex -= 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
    }
}


static void RightBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex < GetMaxAddressIndex() - 5) {
            g_showIndex += 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_ethereumReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
        }
    }
}


static void ChangePathCheckHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkBox;

    if (code == LV_EVENT_CLICKED) {
        checkBox = lv_event_get_target(e);
        for (uint32_t i = 0; i < 3; i++) {
            if (checkBox == g_ethereumReceiveWidgets.changePathWidgets[i].checkBox) {
                lv_obj_add_state(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_clear_flag(g_ethereumReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_ethereumReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                if (g_pathIndex[GetCurrentAccountIndex()] != i) {
                    g_pathIndex[GetCurrentAccountIndex()] = i;
                    g_selectIndex[GetCurrentAccountIndex()] = 0;
                    g_showIndex = 0;
                    RefreshDefaultAddress();
                }
            } else {
                lv_obj_clear_state(g_ethereumReceiveWidgets.changePathWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_add_flag(g_ethereumReceiveWidgets.changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_ethereumReceiveWidgets.changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
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
            if (checkBox == g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox) {
                lv_obj_add_state(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                g_selectIndex[GetCurrentAccountIndex()] = g_showIndex + i;
            } else {
                lv_obj_clear_state(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_add_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_ethereumReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void SelectAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiEthereumReceiveGotoTile(RECEIVE_TILE_SWITCH_ACCOUNT);
        RefreshSwitchAccount();
    }
}

static void GetPathItemSubTittle(char* subTittle, int index)
{
    switch (index) {
    case 0:
        sprintf(subTittle, "m/44'/60'/0'/0/#F5870A X#");
        break;
    case 1:
        sprintf(subTittle, "m/44'/60'/#F5870A X#'/0/0");
        break;
    case 2:
        sprintf(subTittle, "m/44'/60'/0'/#F5870A X#");
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


static void GetHdPath(char *hdPath, int index)
{
    switch (g_pathIndex[GetCurrentAccountIndex()]) {
    case 0:
        sprintf(hdPath, "%s/0/%u", g_paths[g_pathIndex[GetCurrentAccountIndex()]].path, index);
        break;
    case 1:
        sprintf(hdPath, "%s/%u'/0/0", g_paths[g_pathIndex[GetCurrentAccountIndex()]].path, index);
        break;
    case 2:
        sprintf(hdPath, "%s/%u", g_paths[g_pathIndex[GetCurrentAccountIndex()]].path, index);
        break;
    default:
        break;
    }
}

static void GetRootPath(char *rootPath, int index)
{
    switch (g_pathIndex[GetCurrentAccountIndex()]) {
    case 0:
        sprintf(rootPath, "%s", g_paths[g_pathIndex[GetCurrentAccountIndex()]].path);
        break;
    case 1:
        sprintf(rootPath, "%s/%u'", g_paths[g_pathIndex[GetCurrentAccountIndex()]].path, index);
        break;
    case 2:
        sprintf(rootPath, "%s", g_paths[g_pathIndex[GetCurrentAccountIndex()]].path);
        break;
    default:
        break;
    }
}

static char *GetXpub(int index)
{
    switch (g_pathIndex[GetCurrentAccountIndex()]) {
    case 0:
        return GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    case 1:
        return GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LIVE_0 + index);
    case 2:
        return GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY);
    default:
        break;
    }
    return GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);

}

#ifdef COMPILE_SIMULATOR

static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item)
{
    char hdPath[128];
    //sprintf(hdPath, "m/44'/0'/0'/0/%u", index);
    sprintf(hdPath, "%s/0/%u", g_paths[g_pathIndex[GetCurrentAccountIndex()]].path, index);
    printf("hdPath=%s\r\n", hdPath);
    item->index = index;
    sprintf(item->address, "tb1qkcp7vdhczgk5eh59d2l0dxvmpzhx%010u", index);
    strcpy(item->path, hdPath);
}

#else

static void ModelGetEthAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub, hdPath[128], rootPath[128];
    GetHdPath(hdPath, index);
    GetRootPath(rootPath, index);
    xPub = GetXpub(index);
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

void GuiResetCurrentEthAddressIndex(void)
{
    g_selectIndex[GetCurrentAccountIndex()] = 0;
    g_pathIndex[GetCurrentAccountIndex()] = 0;
}

void GuiResetAllEthAddressIndex(void)
{
    memset(g_selectIndex, 0, sizeof(g_selectIndex));
    memset(g_pathIndex, 0, sizeof(g_pathIndex));
}