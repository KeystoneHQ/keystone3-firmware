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
#include "account_manager.h"
#include "gui_animating_qrcode.h"
#include "gui_global_resources.h"
#include "gui_page.h"
#include "keystore.h"
#include "gui_select_address_widgets.h"
#include "account_public_info.h"

#define DERIVATION_PATH_EG_LEN 2

typedef enum {
    CONNECT_WALLET_SELECT_WALLET = 0,
    CONNECT_WALLET_QRCODE,

    CONNECT_WALLET_BUTT,
} CONNECT_WALLET_ENUM;

typedef struct {
    int8_t                  index;
    bool                    state;
    lv_obj_t                *checkBox;
    lv_obj_t                *uncheckedImg;
    lv_obj_t                *checkedImg;
} CoinState_t;

typedef struct ConnectWalletWidget {
    uint32_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    WALLET_LIST_INDEX_ENUM walletIndex;
    lv_obj_t *qrCode;
} ConnectWalletWidget_t;

WalletListItem_t g_walletListArray[] = {
    {WALLET_LIST_BLUE, &walletBluewallet, "Blue Wallet", NULL, 1, true, false},
    {WALLET_LIST_SPARROW, &walletSparrow, "Sparrow", NULL, 1, true, false},
    {WALLET_LIST_NUNCHUK, &walletNunchuk, "Nunchuk", NULL, 1, true, false},
    {WALLET_LIST_ZEUS, &walletZeus, "Zeus", NULL, 1, true, false},
    {WALLET_LIST_BITCOIN_SAFE, &walletBtcSafe, "Bitcoin Safe", NULL, 1, true, false},
    // {WALLET_LIST_SPECTER, &walletSpecter, "Specter", NULL, 1, true, true},
    // {WALLET_LIST_UNISAT, &walletUniSat, "UniSat", NULL, 5, true, true},
};

static lv_obj_t *g_noticeWindow = NULL;
static ConnectWalletWidget_t g_connectWalletTileView;
static PageWidget_t *g_pageWidget;

static void OpenQRCodeHandler(lv_event_t *e);
void ConnectWalletReturnHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);
static lv_obj_t *GuiCreateWalletListItem(lv_obj_t *parent, WalletListItem_t *item, lv_coord_t yPos);

static lv_obj_t *g_coinCont = NULL;
static lv_obj_t *g_coinTitleLabel = NULL;
static lv_obj_t *g_openMoreHintBox = NULL;
static lv_obj_t *g_bottomCont = NULL;
static bool g_isCoinReselected = false;
static lv_obj_t *g_derivationPathCont = NULL;
static void QRCodePause(bool);

static void GuiInitWalletListArray()
{
    int currentWalletIndex = GetCurrentWalletIndex();
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        bool enable = true;
        int index = g_walletListArray[i].index;

        if (currentWalletIndex != SINGLE_WALLET) {
            if (index == WALLET_LIST_SPECTER ||
                    index == WALLET_LIST_UNISAT ||
                    index == WALLET_LIST_ZEUS) {
                enable = false;
            }
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

static void GuiCreateSelectWalletWidget(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    line = GuiCreateLine(parent, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 0);

    lv_coord_t yOffset = 9;
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++) {
        if (!g_walletListArray[i].enable) {
            continue;
        }
        // temporary fix, when the multi-signature testnet is opened, the logic here
        // needs to be rewritten
        if (GetCurrentWalletIndex() == SINGLE_WALLET && GetIsTestNet() &&
                g_walletListArray[i].index == WALLET_LIST_BLUE) {
            continue;
        }

        lv_obj_t *item = GuiCreateWalletListItem(parent, &g_walletListArray[i], yOffset);
        yOffset += 90;

        line = GuiCreateLine(parent, points, 2);
        lv_obj_align_to(line, item, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_scan"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 408);
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
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        lv_obj_t *button = GuiCreateImgLabelAdaptButton(parent, _("multisig_connect_wallet_notice"), &imgTwoSmallKey, UnHandler, NULL);
        lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -24);
        lv_obj_set_style_text_opa(lv_obj_get_child(button, 1), LV_OPA_80, LV_PART_MAIN);
    }
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

    g_connectWalletTileView.currentTile = CONNECT_WALLET_SELECT_WALLET;
    g_connectWalletTileView.tileView = tileView;
    g_connectWalletTileView.cont = cont;

    lv_obj_set_tile_id(g_connectWalletTileView.tileView,
                       g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
}

void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index)
{
    GenerateUR func = NULL;
    SetWallet(g_pageWidget->navBarWidget, index, NULL);
    switch (index) {
    case WALLET_LIST_BLUE:
    case WALLET_LIST_NUNCHUK:
        // 84 49 44
        func = GuiGetStandardBtcData;
        break;
    case WALLET_LIST_ZEUS:
    case WALLET_LIST_SPARROW:
    case WALLET_LIST_BITCOIN_SAFE:
        // 84 49 44 86
        func = GuiGetStandardBtcData;
        break;
    case WALLET_LIST_SPECTER:
        // 84 49
        func = GuiGetSpecterWalletBtcData;
        break;
    case WALLET_LIST_UNISAT:
        func = GuiGetStandardBtcData;
        break;
    default:
        return;
    }
    if (func) {
        GuiAnimatingQRCodeInit(g_connectWalletTileView.qrCode, func, true);
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

static void OpenTutorialHandler(lv_event_t *e)
{
    QRCodePause(true);

    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(wallet));
    GUI_DEL_OBJ(g_openMoreHintBox);
}

static void OpenMoreHandler(lv_event_t *e)
{
    int hintboxHeight = 132;
    lv_obj_t *btn = NULL;
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0),
                        CloseHintBoxHandler, LV_EVENT_CLICKED,
                        &g_openMoreHintBox);
    btn = GuiCreateSelectButton(g_openMoreHintBox, _("Tutorial"), &imgTutorial,
                                OpenTutorialHandler, wallet, true);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
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
    case CONNECT_WALLET_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN,
                         ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                          OpenMoreHandler, &g_connectWalletTileView.walletIndex);
        SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex,
                  NULL);
        QRCodePause(false);
    }
}

void GuiConnectWalletDeInit(void)
{
    GUI_DEL_OBJ(g_openMoreHintBox)
    GUI_DEL_OBJ(g_coinCont)
    GUI_DEL_OBJ(g_derivationPathCont)
    GUI_DEL_OBJ(g_noticeWindow)
    CloseToTargetTileView(g_connectWalletTileView.currentTile,
                          CONNECT_WALLET_SELECT_WALLET);
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

static lv_obj_t *GuiCreateWalletListItem(lv_obj_t *parent, WalletListItem_t *item, lv_coord_t yPos)
{
    GuiButton_t table[] = {
        {.obj = GuiCreateImg(parent, item->walletIcon), .align = LV_ALIGN_LEFT_MID, .position = {20, 0},},
        {.obj = GuiCreateTextLabel(parent, item->walletName), .align = LV_ALIGN_LEFT_MID, .position = {88, 0},},
        {.obj = GuiCreateImg(parent, &imgArrowRight), .align = LV_ALIGN_RIGHT_MID, .position = {-24, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 82, table, NUMBER_OF_ARRAYS(table),
                                       OpenQRCodeHandler, item);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, yPos);
    lv_obj_set_style_bg_opa(button, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(button, 0, LV_PART_MAIN);
    lv_obj_add_flag(button, LV_OBJ_FLAG_CLICKABLE);

    return button;
}