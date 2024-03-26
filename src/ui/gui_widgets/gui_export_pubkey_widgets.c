#include "gui_export_pubkey_widgets.h"
#include "gui_home_widgets.h"
#include "gui_page.h"
#include "gui_fullscreen_mode.h"
#include "account_public_info.h"
#include "assert.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

typedef enum {
    TILEVIEW_QRCODE = 0,
    TILEVIEW_SELECT_TYPE,
} TILEVIEW_INDEX_ENUM;

typedef struct {
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} SelectItem_t;

typedef struct {
    lv_obj_t *cont;
    lv_obj_t *tileview;
    lv_obj_t *qrTileview;
    lv_obj_t *typeTileview;
    lv_obj_t *qrCont;
    lv_obj_t *qrCode;
    lv_obj_t *qrCodeFullscreen;
    lv_obj_t *title;
    lv_obj_t *desc;
    lv_obj_t *pubkey;
    lv_obj_t *egs[2];
    lv_obj_t *confirmBtn;
    SelectItem_t selectItems[4];
} ExportPubkeyWidgets_t;

typedef struct {
    char *title;
    char *subTitle;
    char *path;
    ChainType pubkeyType;
} PathTypeItem_t;

void GetExportPubkey(char *dest, uint16_t chain, uint8_t pathType, uint32_t maxLen);
void CutAndFormatAddress(char *out, uint32_t maxLen, const char *address, uint32_t targetLen);

static void GuiCreateQrCodeWidget(lv_obj_t *parent);
static void OpenSwitchPathTypeHandler(lv_event_t *e);
static void RefreshQrcode();
static void CloseSwitchPathTypeHandler(lv_event_t *e);
static uint8_t GetPathType();
static void SetEgContent(uint8_t index);
static void GuiCreateSwitchPathTypeWidget(lv_obj_t *parent);
static void SelectItemCheckHandler(lv_event_t *e);
static void RefreshPathType();
static void ConfirmHandler(lv_event_t *e);
static void UpdateConfirmBtn(void);
static void SetPathType(uint8_t pathType);
static void SetCheckboxState(uint8_t i, bool isChecked);

static const PathTypeItem_t g_btcPathTypeList[] = {
    {"Native SegWit", "P2WPKH",      "m/84'/0'/0'", XPUB_TYPE_BTC_NATIVE_SEGWIT},
    {"Taproot",       "P2TR",        "m/86'/0'/0'", XPUB_TYPE_BTC_TAPROOT      },
    {"Nested SegWit", "P2SH-P2WPKH", "m/49'/0'/0'", XPUB_TYPE_BTC              },
    {"Legacy",        "P2PKH",       "m/44'/0'/0'", XPUB_TYPE_BTC_LEGACY       },
};

#ifdef BTC_ONLY
static const char *g_btcTestNetPath[] = {
    "m/86'/1'/0'",
    "m/84'/1'/0'",
    "m/49'/1'/0'",
    "m/44'/1'/0'",
};
#endif

static GuiChainCoinType g_chain;
static PageWidget_t *g_pageWidget;
static ExportPubkeyWidgets_t g_widgets;
static TILEVIEW_INDEX_ENUM g_tileviewIndex;
static uint8_t g_btcPathType[3] = {0};
static uint8_t g_tmpSelectIndex = 0;

static void GuiRefreshTileview()
{
    switch (g_tileviewIndex) {
    case TILEVIEW_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseTimerCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("receive_btc_extended_public_key"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        RefreshQrcode();
        break;

    case TILEVIEW_SELECT_TYPE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseSwitchPathTypeHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("receive_btc_address_type"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        RefreshPathType();
        break;

    default:
        break;
    }
}

static void GuiGotoTileview(TILEVIEW_INDEX_ENUM index)
{
    g_tileviewIndex = index;
    lv_obj_set_tile_id(g_widgets.tileview, g_tileviewIndex, 0, LV_ANIM_OFF);
    GuiRefreshTileview();
}

void GuiExportPubkeyInit(uint8_t chain)
{
    g_chain = chain;
    g_pageWidget = CreatePageWidget();
    g_widgets.cont = g_pageWidget->contentZone;

    g_widgets.tileview = lv_tileview_create(g_widgets.cont);
    lv_obj_set_style_bg_opa(g_widgets.tileview, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_widgets.tileview, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_clear_flag(g_widgets.tileview, LV_OBJ_FLAG_SCROLLABLE);

    g_widgets.qrTileview = lv_tileview_add_tile(g_widgets.tileview, TILEVIEW_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_widgets.qrTileview);

    g_widgets.typeTileview = lv_tileview_add_tile(g_widgets.tileview, TILEVIEW_SELECT_TYPE, 0, LV_DIR_HOR);
    GuiCreateSwitchPathTypeWidget(g_widgets.typeTileview);

    GuiGotoTileview(TILEVIEW_QRCODE);
}

void GuiExportPubkeyDeInit(void)
{
    CLEAR_OBJECT(g_widgets);
    GuiFullscreenModeCleanUp();

    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

lv_obj_t* CreateExportPubkeyQRCode(lv_obj_t* parent, uint16_t w, uint16_t h)
{
    lv_obj_t* qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    char pubkey[BUFFER_SIZE_128] = {0};
    GetExportPubkey(pubkey, g_chain, GetPathType(), sizeof(pubkey));
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    lv_qrcode_update(qrcode, pubkey, strnlen_s(pubkey, BUFFER_SIZE_128) + 1);
    return qrcode;
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *tempObj;
    uint16_t yOffset = 0;

    g_widgets.qrCont = GuiCreateContainerWithParent(parent, 408, 634);
    lv_obj_align(g_widgets.qrCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_widgets.qrCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_widgets.qrCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(g_widgets.qrCont, 24, LV_PART_MAIN);

    yOffset += 12;
    lv_obj_t *btn = lv_btn_create(g_widgets.qrCont);
    lv_obj_set_size(btn, 384, 96);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 12, yOffset);
    lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, OpenSwitchPathTypeHandler, LV_EVENT_CLICKED, NULL);
    yOffset += 96;

    tempObj = GuiCreateImg(btn, &imgArrowRight);
    lv_obj_align(tempObj, LV_ALIGN_RIGHT_MID, -24, 0);

    g_widgets.title = GuiCreateBoldIllustrateLabel(btn, "");
    lv_obj_align(g_widgets.title, LV_ALIGN_TOP_LEFT, 24, 16);

    g_widgets.desc = GuiCreateNoticeLabel(btn, "");
    lv_obj_align(g_widgets.desc, LV_ALIGN_TOP_LEFT, 24, 50);

    yOffset += 8;
    g_widgets.qrCode = CreateExportPubkeyQRCode(g_widgets.qrCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    g_widgets.qrCodeFullscreen = GuiFullscreenModeCreateObject(CreateExportPubkeyQRCode, 420, 420);
    lv_obj_align(g_widgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 8;
    g_widgets.pubkey = GuiCreateNoticeLabel(g_widgets.qrCont, "");
    lv_obj_set_width(g_widgets.pubkey, 336);
    lv_obj_align(g_widgets.pubkey, LV_ALIGN_TOP_MID, 0, yOffset);
}

static void GuiCreateSwitchPathTypeWidget(lv_obj_t *parent)
{
    lv_obj_t *cont, *line, *label;
    char desc[BUFFER_SIZE_64] = {0};
#ifdef BTC_ONLY
    const char *path;
#endif
    static lv_point_t points[2] = {{0, 0}, {360, 0}};

    cont = GuiCreateContainerWithParent(parent, 408, 411);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    for (uint32_t i = 0; i < sizeof(g_btcPathTypeList) / sizeof(g_btcPathTypeList[0]); i++) {
        label = GuiCreateLabelWithFont(cont, g_btcPathTypeList[i].title, &openSans_24);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
#ifndef BTC_ONLY
        snprintf_s(desc, BUFFER_SIZE_64, "%s (%s)", g_btcPathTypeList[i].subTitle, g_btcPathTypeList[i].path);
#else
        path = GetIsTestNet() ? g_btcTestNetPath[i] : g_btcPathTypeList[i].path;
        snprintf_s(desc, BUFFER_SIZE_64, "%s (%s)", g_btcPathTypeList[i].subTitle, path);
#endif
        label = GuiCreateNoticeLabel(cont, desc);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i != (sizeof(g_btcPathTypeList) / sizeof(g_btcPathTypeList[0]) - 1)) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * (i + 1));
        }
        g_widgets.selectItems[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_widgets.selectItems[i].checkBox, 408, 82);
        lv_obj_align(g_widgets.selectItems[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_widgets.selectItems[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_widgets.selectItems[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_widgets.selectItems[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_widgets.selectItems[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_widgets.selectItems[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_widgets.selectItems[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_widgets.selectItems[i].checkBox, SelectItemCheckHandler, LV_EVENT_CLICKED, NULL);

        g_widgets.selectItems[i].checkedImg = GuiCreateImg(g_widgets.selectItems[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_widgets.selectItems[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_widgets.selectItems[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_widgets.selectItems[i].uncheckedImg = GuiCreateImg(g_widgets.selectItems[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_widgets.selectItems[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_widgets.selectItems[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_clear_flag(g_widgets.selectItems[g_tmpSelectIndex].checkedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_widgets.selectItems[g_tmpSelectIndex].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *egCont = GuiCreateContainerWithParent(parent, 408, 122);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);

    label = GuiCreateNoticeLabel(egCont, _("derivation_path_address_eg"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    for (uint32_t i = 0; i < 2; i++) {
        label = GuiCreateLabel(egCont, "");
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 50 + 34 * i);
        lv_obj_set_width(label, 360);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_label_set_recolor(label, true);
        g_widgets.egs[i] = label;
    }

    lv_obj_t *btn = GuiCreateBtn(parent, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, ConfirmHandler, LV_EVENT_CLICKED, NULL);
    g_widgets.confirmBtn = btn;
}

static void OpenSwitchPathTypeHandler(lv_event_t *e)
{
    GuiGotoTileview(TILEVIEW_SELECT_TYPE);
}

static void CloseSwitchPathTypeHandler(lv_event_t *e)
{
    GuiGotoTileview(TILEVIEW_QRCODE);
}

static void SelectItemCheckHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkBox;

    if (code == LV_EVENT_CLICKED) {
        checkBox = lv_event_get_target(e);
        for (uint32_t i = 0; i < sizeof(g_btcPathTypeList) / sizeof(g_btcPathTypeList[0]); i++) {
            if (checkBox == g_widgets.selectItems[i].checkBox) {
                SetCheckboxState(i, true);
                g_tmpSelectIndex = i;
                SetEgContent(i);
                UpdateConfirmBtn();
            } else {
                SetCheckboxState(i, false);
            }
        }
    }
}

static void ConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED && g_tmpSelectIndex != GetPathType()) {
        SetPathType(g_tmpSelectIndex);
        GuiGotoTileview(TILEVIEW_QRCODE);
    }
}

#ifdef BTC_ONLY
static ChainType ConvertChainType(ChainType chainType)
{
    switch (chainType) {
    case XPUB_TYPE_BTC_TAPROOT:
        return GetIsTestNet() ? XPUB_TYPE_BTC_TAPROOT_TEST : XPUB_TYPE_BTC_TAPROOT;
    case XPUB_TYPE_BTC_NATIVE_SEGWIT:
        return GetIsTestNet() ? XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST : XPUB_TYPE_BTC_NATIVE_SEGWIT;
    case XPUB_TYPE_BTC:
        return GetIsTestNet() ? XPUB_TYPE_BTC_TEST : XPUB_TYPE_BTC;
    case XPUB_TYPE_BTC_LEGACY:
        return GetIsTestNet() ? XPUB_TYPE_BTC_LEGACY_TEST : XPUB_TYPE_BTC_LEGACY;
    default:
        break;
    }
    return chainType;
}
#endif
static void GetBtcPubkey(char *dest, uint8_t pathType, uint32_t maxLen)
{
    SimpleResponse_c_char *result;
#ifndef BTC_ONLY
    ChainType chainType = g_btcPathTypeList[pathType].pubkeyType;
#else
    ChainType chainType = ConvertChainType(g_btcPathTypeList[pathType].pubkeyType);
#endif
    char *xpub = GetCurrentAccountPublicKey(chainType);
    char head[] = "ypub";
    switch (chainType) {
    case XPUB_TYPE_BTC_LEGACY:
    case XPUB_TYPE_BTC_TAPROOT:
        strcpy_s(dest, maxLen, xpub);
        return;
    case XPUB_TYPE_BTC_NATIVE_SEGWIT:
        head[0] = 'z';
        break;
#ifdef BTC_ONLY
    case XPUB_TYPE_BTC_LEGACY_TEST:
    case XPUB_TYPE_BTC_TAPROOT_TEST:
        head[0] = 't';
        break;
    case XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST:
        head[0] = 'v';
        break;
    case XPUB_TYPE_BTC_TEST:
        head[0] = 'u';
        break;
#endif
    default:
        break;
    }
    result = xpub_convert_version(xpub, head);
    ASSERT(result);
    strcpy_s(dest, maxLen, result->data);
    free_simple_response_c_char(result);
}

void GetExportPubkey(char *dest, uint16_t chain, uint8_t pathType, uint32_t maxLen)
{
    switch (chain) {
    case CHAIN_BTC:
        GetBtcPubkey(dest, pathType, maxLen);
        break;
    default:
        printf("(GetExportPubkey) unsupported chain type: %d\r\n", g_chain);
    }
}

static char *GetPathTypeTitle(uint16_t chain, uint8_t pathType)
{
    switch (chain) {
    case CHAIN_BTC:
        return g_btcPathTypeList[pathType].title;
    default:
        printf("(GetPathTypeTitle) unsupported chain type: %d\r\n", chain);
        return NULL;
    }
}

#ifndef BTC_ONLY
static void GetPathTypeDesc(char *dest, uint16_t chain, uint8_t pathType, uint32_t maxLen)
{
    switch (chain) {
    case CHAIN_BTC:
        snprintf_s(dest, maxLen, "%s (%s)", g_btcPathTypeList[pathType].subTitle, g_btcPathTypeList[pathType].path);
        break;
    default:
        printf("(GetPathTypeDesc) unsupported chain type: %d\r\n", chain);
    }
}
#else
static void GetPathTypeDesc(char *dest, uint16_t chain, uint8_t pathType, uint32_t maxLen)
{
    ASSERT(chain == CHAIN_BTC);
    const char *path = GetIsTestNet() ? g_btcTestNetPath[pathType] : g_btcPathTypeList[pathType].path;
    snprintf_s(dest, maxLen, "%s (%s)", g_btcPathTypeList[pathType].subTitle, path);
}
#endif

static void RefreshQrcode()
{
    uint8_t pathType = GetPathType();
    char pubkey[BUFFER_SIZE_128];
    GetExportPubkey(pubkey, g_chain, pathType, sizeof(pubkey));

    lv_label_set_text(g_widgets.title, GetPathTypeTitle(g_chain, pathType));
    char desc[BUFFER_SIZE_32] = {0};
    GetPathTypeDesc(desc, g_chain, pathType, sizeof(desc));
    lv_label_set_text(g_widgets.desc, desc);
    lv_qrcode_update(g_widgets.qrCode, pubkey, strnlen_s(pubkey, BUFFER_SIZE_128));
    lv_qrcode_update(g_widgets.qrCodeFullscreen, pubkey, strnlen_s(pubkey, BUFFER_SIZE_128));
    lv_label_set_text(g_widgets.pubkey, pubkey);
    lv_obj_update_layout(g_widgets.pubkey);
    lv_obj_set_height(g_widgets.qrCont, lv_obj_get_height(g_widgets.pubkey) + 484);
}

static uint8_t GetPathType()
{
    switch (g_chain) {
    case CHAIN_BTC:
        return g_btcPathType[GetCurrentAccountIndex()];
    default:
        break;
    }
    return 0;
}

static void SetPathType(uint8_t pathType)
{
    switch (g_chain) {
    case CHAIN_BTC:
        g_btcPathType[GetCurrentAccountIndex()] = pathType;
        break;

    default:
        break;
    }
}

#ifndef BTC_ONLY
static void ModelGetUtxoAddress(char *dest, uint8_t pathType, uint32_t index, uint32_t maxLen)
{
    char *xPub, hdPath[BUFFER_SIZE_128];
    xPub = GetCurrentAccountPublicKey(g_btcPathTypeList[pathType].pubkeyType);
    ASSERT(xPub);
    SimpleResponse_c_char *result;
    snprintf_s(hdPath, sizeof(hdPath), "%s/0/%u", g_btcPathTypeList[pathType].path, index);
    do {
        result = utxo_get_address(hdPath, xPub);
        CHECK_CHAIN_BREAK(result);
    } while (0);
    snprintf_s(dest, maxLen, "%s", result->data);
    free_simple_response_c_char(result);
}
#else
static void ModelGetUtxoAddress(char *dest, uint8_t pathType, uint32_t index, uint32_t maxLen)
{
    char *xPub, hdPath[128];
    const char *rootPath;
    ChainType chainType = ConvertChainType(g_btcPathTypeList[pathType].pubkeyType);
    rootPath = GetIsTestNet() ? g_btcTestNetPath[pathType] : g_btcPathTypeList[pathType].path;
    xPub = GetCurrentAccountPublicKey(chainType);
    ASSERT(xPub);
    SimpleResponse_c_char *result;
    snprintf_s(hdPath, sizeof(hdPath), "%s/0/%u", rootPath, index);
    do {
        result = utxo_get_address(hdPath, xPub);
        CHECK_CHAIN_BREAK(result);
    } while (0);
    snprintf_s(dest, maxLen, "%s", result->data);
    free_simple_response_c_char(result);
}
#endif

static void SetEgContent(uint8_t index)
{
    char eg[BUFFER_SIZE_64] = {0};
    char prefix[8] = {0};
    char rest[BUFFER_SIZE_64] = {0};
    char addr[BUFFER_SIZE_128] = {0};
    char addrShot[BUFFER_SIZE_64] = {0};
    int8_t prefixLen = (g_btcPathTypeList[index].pubkeyType == XPUB_TYPE_BTC_NATIVE_SEGWIT || g_btcPathTypeList[index].pubkeyType == XPUB_TYPE_BTC_TAPROOT) ? 4 : 1;
    for (uint8_t i = 0; i < 2; i++) {
        ModelGetUtxoAddress(addr, index, i, sizeof(addr));
        CutAndFormatAddress(addrShot, sizeof(addrShot), addr, 24);
        strncpy(prefix, addrShot, prefixLen);
        strncpy(rest, addrShot + prefixLen, strnlen_s(addrShot, BUFFER_SIZE_64) - prefixLen);
        snprintf_s(eg, sizeof(eg), "%d  #F5870A %s#%s", i, prefix, rest);
        lv_label_set_text(g_widgets.egs[i], eg);
    }
}

static void SetCheckboxState(uint8_t i, bool isChecked)
{
    if (isChecked) {
        lv_obj_add_state(g_widgets.selectItems[i].checkBox, LV_STATE_CHECKED);
        lv_obj_clear_flag(g_widgets.selectItems[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_widgets.selectItems[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_state(g_widgets.selectItems[i].checkBox, LV_STATE_CHECKED);
        lv_obj_add_flag(g_widgets.selectItems[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_widgets.selectItems[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
}

static void RefreshPathType()
{
    g_tmpSelectIndex = GetPathType();
    for (uint8_t i = 0; i < sizeof(g_btcPathTypeList) / sizeof(g_btcPathTypeList[0]); i++) {
        SetCheckboxState(i, i == g_tmpSelectIndex);
    }
    SetEgContent(g_tmpSelectIndex);
    UpdateConfirmBtn();
}

static void UpdateConfirmBtn(void)
{
    if (g_tmpSelectIndex != GetPathType()) {
        lv_obj_set_style_bg_opa(g_widgets.confirmBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_widgets.confirmBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_widgets.confirmBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_widgets.confirmBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}
