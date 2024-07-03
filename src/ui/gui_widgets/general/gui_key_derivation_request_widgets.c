#ifndef BTC_ONLY
#include "gui_key_derivation_request_widgets.h"
#include "gui.h"
#include "gui_page.h"
#include "librust_c.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_button.h"
#include "gui_hintbox.h"

typedef struct KeyDerivationWidget {
    uint8_t currentTile;
    PageWidget_t *pageWidget;
    lv_obj_t *tileView;
    lv_obj_t *cont;
    lv_obj_t *qrCode;
} KeyDerivationWidget_t;

typedef enum {
    TILE_APPROVE,
    TILE_QRCODE,

    TILE_BUTT,
} PAGE_TILE;

static void *g_data;
static URParseResult *g_urResult;
static URParseMultiResult *g_urMultiResult;
static bool g_isMulti;
static KeyDerivationWidget_t g_keyDerivationTileView;
static QRHardwareCallData *g_callData;
static Response_QRHardwareCallData *g_response;
static WALLET_LIST_INDEX_ENUM g_walletIndex;
static lv_obj_t *g_openMoreHintBox;
static void RecalcCurrentWalletIndex(char *origin);

static void GuiCreateApproveWidget(lv_obj_t *parent);
static void GuiCreateQRCodeWidget(lv_obj_t *parent);
static void OnApproveHandler(lv_event_t *e);
static void OnReturnHandler(lv_event_t *e);
static void ModelParseQRHardwareCall();
static UREncodeResult *ModelGenerateSyncUR(void);
static uint8_t GetXPubIndexByPath(char *path);
static void OpenTutorialHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);

void GuiSetKeyDerivationRequestData(void *urResult, void *multiResult, bool is_multi)
{
    g_urResult = urResult;
    g_urMultiResult = multiResult;
    g_isMulti = is_multi;
    g_data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
}

void FreeKeyDerivationRequestMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    if (g_response != NULL) {
        free_Response_QRHardwareCallData(g_response);
        g_response = NULL;
    }
}

static void RecalcCurrentWalletIndex(char *origin)
{
    if (strcmp("eternl", origin) == 0) {
        g_walletIndex = WALLET_LIST_ETERNL;
    } else if (strcmp("Typhon Extension", origin) == 0) {
        g_walletIndex = WALLET_LIST_TYPHON;
    } else {
        g_walletIndex = WALLET_LIST_ETERNL;
    }
}

void GuiKeyDerivationRequestInit()
{
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    g_keyDerivationTileView.pageWidget = CreatePageWidget();
    g_keyDerivationTileView.cont = g_keyDerivationTileView.pageWidget->contentZone;
    SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    lv_obj_t *tileView = lv_tileview_create(g_keyDerivationTileView.cont);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(tileView, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(tileView, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, TILE_APPROVE, 0, LV_DIR_HOR);
    GuiCreateApproveWidget(tile);
    RecalcCurrentWalletIndex(g_response->data->origin);
    SetWallet(g_keyDerivationTileView.pageWidget->navBarWidget, g_walletIndex, NULL);
    SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, NULL);

    tile = lv_tileview_add_tile(tileView, TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQRCodeWidget(tile);

    g_keyDerivationTileView.currentTile = TILE_APPROVE;
    g_keyDerivationTileView.tileView = tileView;

    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}
void GuiKeyDerivationRequestDeInit()
{
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    GuiAnimatingQRCodeDestroyTimer();
    FreeKeyDerivationRequestMemory();
}
void GuiKeyDerivationRequestRefresh()
{
    GuiAnimatingQRCodeControl(false);
}
void GuiKeyDerivationRequestNextTile()
{
    g_keyDerivationTileView.currentTile++;
    switch (g_keyDerivationTileView.currentTile) {
    case TILE_QRCODE:
        SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
        break;
    default:
        break;
    }
    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}
void GuiKeyDerivationRequestPrevTile()
{
    g_keyDerivationTileView.currentTile--;
    switch (g_keyDerivationTileView.currentTile) {
    case TILE_APPROVE:
        SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        GuiAnimatingQRCodeDestroyTimer();
        break;
    default:
        break;
    }
    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}

static void ModelParseQRHardwareCall()
{
    Response_QRHardwareCallData *data = parse_qr_hardware_call(g_data);
    g_callData = data->data;
    g_response = data;
}

static UREncodeResult *ModelGenerateSyncUR(void)
{
    CSliceFFI_ExtendedPublicKey keys;
    ExtendedPublicKey xpubs[24];
    for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
        KeyDerivationSchema schema = g_callData->key_derivation->schemas->data[i];
        char* xpub = GetCurrentAccountPublicKey(GetXPubIndexByPath(schema.key_path));
        xpubs[i].path = schema.key_path;
        xpubs[i].xpub = xpub;
    }
    keys.data = xpubs;
    keys.size = g_callData->key_derivation->schemas->size;
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    return generate_key_derivation_ur(mfp, 4, &keys);
}


static uint8_t GetXPubIndexByPath(char *path)
{
    if (strcmp("1852'/1815'/1'", path) == 0) return XPUB_TYPE_ADA_1;
    if (strcmp("1852'/1815'/2'", path) == 0) return XPUB_TYPE_ADA_2;
    if (strcmp("1852'/1815'/3'", path) == 0) return XPUB_TYPE_ADA_3;
    if (strcmp("1852'/1815'/4'", path) == 0) return XPUB_TYPE_ADA_4;
    if (strcmp("1852'/1815'/5'", path) == 0) return XPUB_TYPE_ADA_5;
    if (strcmp("1852'/1815'/6'", path) == 0) return XPUB_TYPE_ADA_6;
    if (strcmp("1852'/1815'/7'", path) == 0) return XPUB_TYPE_ADA_7;
    if (strcmp("1852'/1815'/8'", path) == 0) return XPUB_TYPE_ADA_8;
    if (strcmp("1852'/1815'/9'", path) == 0) return XPUB_TYPE_ADA_9;
    if (strcmp("1852'/1815'/10'", path) == 0) return XPUB_TYPE_ADA_10;
    if (strcmp("1852'/1815'/11'", path) == 0) return XPUB_TYPE_ADA_11;
    if (strcmp("1852'/1815'/12'", path) == 0) return XPUB_TYPE_ADA_12;
    if (strcmp("1852'/1815'/13'", path) == 0) return XPUB_TYPE_ADA_13;
    if (strcmp("1852'/1815'/14'", path) == 0) return XPUB_TYPE_ADA_14;
    if (strcmp("1852'/1815'/15'", path) == 0) return XPUB_TYPE_ADA_15;
    if (strcmp("1852'/1815'/16'", path) == 0) return XPUB_TYPE_ADA_16;
    if (strcmp("1852'/1815'/17'", path) == 0) return XPUB_TYPE_ADA_17;
    if (strcmp("1852'/1815'/18'", path) == 0) return XPUB_TYPE_ADA_18;
    if (strcmp("1852'/1815'/19'", path) == 0) return XPUB_TYPE_ADA_19;
    if (strcmp("1852'/1815'/20'", path) == 0) return XPUB_TYPE_ADA_20;
    if (strcmp("1852'/1815'/21'", path) == 0) return XPUB_TYPE_ADA_21;
    if (strcmp("1852'/1815'/22'", path) == 0) return XPUB_TYPE_ADA_22;
    if (strcmp("1852'/1815'/23'", path) == 0) return XPUB_TYPE_ADA_23;
    return XPUB_TYPE_ADA_0;
}

static void GuiCreateApproveWidget(lv_obj_t *parent)
{
    ModelParseQRHardwareCall();

    lv_obj_t *label, *cont, *btn, *pathCont;

    cont = GuiCreateContainerWithParent(parent, 408, 534);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 36, 8);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    label = GuiCreateIllustrateLabel(cont, _("connect_wallet_key_request_fmt"));
    lv_label_set_text_fmt(label, _("connect_wallet_key_request_fmt"), g_response->data->origin);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    pathCont = GuiCreateContainerWithParent(cont, 408, 450);
    lv_obj_align(pathCont, LV_ALIGN_TOP_LEFT, 0, 92);
    lv_obj_set_style_radius(pathCont, 24, 0);
    lv_obj_set_style_bg_color(pathCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pathCont, LV_OPA_12, LV_PART_MAIN);
    lv_obj_add_flag(pathCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(pathCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(pathCont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};

    for (size_t i = 0; i < g_response->data->key_derivation->schemas->size; i++) {
        cont = GuiCreateContainerWithParent(pathCont, 408, 102);
        lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 102 * i);
        lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_MAIN);
        char title[BUFFER_SIZE_32] = {0};
        sprintf(title, "%s-%d", _("account_head"), i);
        label = GuiCreateIllustrateLabel(cont, title);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
        char path[BUFFER_SIZE_64] = {0};
        snprintf_s(path, BUFFER_SIZE_64, "M/%s", g_response->data->key_derivation->schemas->data[i].key_path);
        label = GuiCreateIllustrateLabel(cont, path);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 101);
        }
    }

    cont = GuiCreateContainerWithParent(parent, 480, 114);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    btn = GuiCreateTextBtn(cont, _("Cancel"));
    lv_obj_set_size(btn, 192, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 36, 24);
    lv_obj_add_event_cb(btn, CloseCurrentViewHandler, LV_EVENT_CLICKED, NULL);

    btn = GuiCreateTextBtn(cont, _("Approve"));
    lv_obj_set_size(btn, 192, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 252, 24);
    lv_obj_add_event_cb(btn, OnApproveHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiCreateQRCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_scan"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 482);
    lv_obj_add_flag(qrCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(qrCont, 336, 336);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_color(qrBgCont, WHITE_COLOR, LV_PART_MAIN);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(qrBgCont, 294, 294);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);

    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);
    g_keyDerivationTileView.qrCode = qrcode;

    lv_obj_t *bottomCont = GuiCreateContainerWithParent(qrCont, 408, 104);
    lv_obj_align(bottomCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bottomCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bottomCont, LV_OPA_0, LV_STATE_DEFAULT | LV_PART_MAIN);

    label = GuiCreateNoticeLabel(bottomCont, _("Network"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    lv_obj_t *coinCont = GuiCreateContainerWithParent(bottomCont, 280, 30);
    lv_obj_align(coinCont, LV_ALIGN_TOP_LEFT, 36, 50);
    lv_obj_set_style_bg_color(coinCont, DARK_BG_COLOR, LV_PART_MAIN);

    lv_obj_t *img = GuiCreateImg(coinCont, &coinAda);
    lv_img_set_zoom(img, 110);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);
}

static void OnApproveHandler(lv_event_t *e)
{
    GuiAnimatingQRCodeInit(g_keyDerivationTileView.qrCode, ModelGenerateSyncUR, true);
    GuiKeyDerivationRequestNextTile();
}

static void OnReturnHandler(lv_event_t *e)
{
    GuiKeyDerivationRequestPrevTile();
}

void GuiKeyDerivationWidgetHandleURGenerate(char *data, uint16_t len)
{
    GuiAnimantingQRCodeFirstUpdate(data, len);
}

void GuiKeyDerivationWidgetHandleURUpdate(char *data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
}

static void OpenMoreHandler(lv_event_t *e)
{
    int hintboxHeight = 132;
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_openMoreHintBox);
    lv_obj_t *label = GuiCreateTextLabel(g_openMoreHintBox, _("Tutorial"));
    lv_obj_t *img = GuiCreateImg(g_openMoreHintBox, &imgTutorial);

    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {76, 0},
        },
    };
    lv_obj_t *btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                    OpenTutorialHandler, &g_walletIndex);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
}

static void OpenTutorialHandler(lv_event_t *e)
{
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(WALLET_LIST_INDEX_ENUM));
    GUI_DEL_OBJ(g_openMoreHintBox);
}
#endif
