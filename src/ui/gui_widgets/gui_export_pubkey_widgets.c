#include "gui_chain.h"
#include "gui_obj.h"
#include "gui_export_pubkey_widgets.h"
#include "gui_home_widgets.h"
#include "gui_page.h"
#include "gui_fullscreen_mode.h"
#include "account_public_info.h"
#include "assert.h"
#include "gui_hintbox.h"
#include "sdcard_manager.h"

#ifdef BTC_ONLY
#include "gui_animating_qrcode.h"
#include "keystore.h"
#include "gui_home_widgets.h"
static lv_obj_t *g_noticeWindow = NULL;
static char *g_xpubConfigName = NULL;
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
    "m/84'/1'/0'",
    "m/86'/1'/0'",
    "m/49'/1'/0'",
    "m/44'/1'/0'",
};

static const PathTypeItem_t g_btcMultisigPathList[] = {
    {"Native SegWit", "P2WSH", "m/48'/0'/0'/2'", XPUB_TYPE_BTC_MULTI_SIG_P2WSH},
    {"Nested SegWit", "P2WSH-P2SH", "m/48'/0'/0'/1'", XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH},
    {"Legacy", "P2SH", "m/45'", XPUB_TYPE_BTC_MULTI_SIG_P2SH},
};
#endif

static GuiChainCoinType g_chain;
static PageWidget_t *g_pageWidget;
static ExportPubkeyWidgets_t g_widgets;
static TILEVIEW_INDEX_ENUM g_tileviewIndex;
static uint8_t g_btcPathType[3] = {0};
static uint8_t g_tmpSelectIndex = 0;
static PathTypeItem_t *g_pathTypeList = NULL;
static uint8_t g_btcPathNum = 0;
static bool g_isTest = false;
static bool g_isMultisig = false;

void OpenExportViewHandler(lv_event_t *e)
{
    static HOME_WALLET_CARD_ENUM chainCard = HOME_WALLET_CARD_BTC;

    g_isTest = *(bool *)lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_exportPubkeyView, &chainCard, sizeof(chainCard));
}

#ifdef BTC_ONLY
void OpenExportMultisigViewHandler(lv_event_t *e)
{
    static HOME_WALLET_CARD_ENUM chainCard = HOME_WALLET_CARD_BTC_MULTISIG;

    g_isMultisig = true;
    GuiFrameOpenViewWithParam(&g_exportPubkeyView, &chainCard, sizeof(chainCard));
}

static int CreateExportPubkeyComponent(char *xpub, uint32_t maxLen)
{
    char *derivHead[3] = {
        "p2wsh_deriv",
        "p2sh_p2wsh_deriv",
        "p2sh_deriv"
    };
    char *xpubHead[3] = {
        "p2wsh",
        "p2sh_p2wsh",
        "p2sh"
    };
    int len = 0;
    len += snprintf_s(xpub + len, maxLen - len, "{\n");
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_btcMultisigPathList); i++) {
        len += snprintf_s(xpub + len, maxLen - len, "  \"%s\": \"%s\",\n", derivHead[i], g_btcMultisigPathList[i].path);
        len += snprintf_s(xpub + len, maxLen - len, "  \"%s\": \"%s\",\n", xpubHead[i], GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH - i));
    }

    len += snprintf_s(xpub + len, maxLen - len, "  \"account\": \"0\",\n");
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    len += snprintf_s(xpub + len, maxLen - len, "  \"xfp\": \"%02X%02X%02X%02X\"\n", mfp[0], mfp[1], mfp[2], mfp[3]);
    len += snprintf_s(xpub + len, maxLen - len, "}");
    return len;
}

static void GuiWriteToMicroCardHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)

    char *xPubBuff = EXT_MALLOC(1024);
    int len = CreateExportPubkeyComponent(xPubBuff, 1024);
    int ret = FileWrite(g_xpubConfigName, xPubBuff, len);
    if (ret) {
        g_noticeWindow =  GuiCreateErrorCodeWindow(ERR_EXPORT_FILE_TO_MICRO_CARD_FAILED, &g_noticeWindow, NULL);
    }
    EXT_FREE(xPubBuff);
}

static void GuiExportXpubToMicroCard(void)
{
    g_noticeWindow = GuiCreateConfirmHintBox(&imgSdCardL, _("wallet_profile_export_to_sdcard_title"), _("about_info_export_file_name"), g_xpubConfigName, _("got_it"), ORANGE_COLOR);
    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(btn, GuiWriteToMicroCardHandler, LV_EVENT_CLICKED, NULL);
}

static UREncodeResult *GuiGenerateUR()
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[1];
    public_keys->data = keys;
    public_keys->size = 1;
    keys[0].path = g_btcMultisigPathList[GetPathType()].path;
    keys[0].xpub = GetCurrentAccountPublicKey(g_btcMultisigPathList[GetPathType()].pubkeyType);

    struct UREncodeResult *result = export_multi_sig_xpub_by_ur(mfp, sizeof(mfp), public_keys, MainNet);
    SRAM_FREE(public_keys);
    return result;
}
#endif

static void GuiRefreshTileview()
{
    switch (g_tileviewIndex) {
    case TILEVIEW_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseTimerCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("receive_btc_extended_public_key"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        RefreshQrcode();
#ifdef BTC_ONLY
        if (g_isMultisig) {
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SDCARD, GuiSDCardExportHandler, GuiExportXpubToMicroCard);
        }
#endif
        break;

    case TILEVIEW_SELECT_TYPE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseSwitchPathTypeHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("receive_btc_address_type"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
#ifdef BTC_ONLY
        if (g_isMultisig) {
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("connect_wallet_xpub_script_format"));
        }
#endif
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

static void InitPathAndChain(uint8_t chain)
{
#ifdef BTC_ONLY
    if (chain == HOME_WALLET_CARD_BTC) {
        g_chain = CHAIN_BTC;
        g_pathTypeList = (PathTypeItem_t *)g_btcPathTypeList;
        g_btcPathNum = NUMBER_OF_ARRAYS(g_btcPathTypeList);
    } else if (chain == HOME_WALLET_CARD_BTC_MULTISIG) {
        g_chain = CHAIN_BTC;
        g_pathTypeList = (PathTypeItem_t *)g_btcMultisigPathList;
        g_btcPathNum = NUMBER_OF_ARRAYS(g_btcMultisigPathList);
    } else {
        g_chain = chain;
    }
    g_xpubConfigName = SRAM_MALLOC(BUFFER_SIZE_64);
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    snprintf_s(g_xpubConfigName, BUFFER_SIZE_64, "%s-%02X%02X%02X%02X.json", GetWalletName(), mfp[0], mfp[1], mfp[2], mfp[3]);
#else
    g_chain = chain;
    g_pathTypeList = (PathTypeItem_t *)g_btcPathTypeList;
    g_btcPathNum = NUMBER_OF_ARRAYS(g_btcPathTypeList);
#endif
}

void GuiExportPubkeyInit(uint8_t chain)
{
    InitPathAndChain(chain);
    g_pageWidget = CreatePageWidget();
    g_widgets.cont = g_pageWidget->contentZone;

    g_widgets.tileview = GuiCreateTileView(g_widgets.cont);
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
    g_btcPathNum = 0;
    g_tmpSelectIndex = 0;
    g_isTest = false;

#ifdef BTC_ONLY
    g_isMultisig = false;
    GuiAnimatingQRCodeDestroyTimer();
    GUI_DEL_OBJ(g_noticeWindow)
    SRAM_FREE(g_xpubConfigName);
#endif
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

#ifdef BTC_ONLY
    if (g_isMultisig) {
        lv_obj_t *label = GuiCreateNoticeLabel(btn, "");
        uint8_t mfp[4];
        GetMasterFingerPrint(mfp);
        lv_label_set_text_fmt(label, "%02X%02X%02X%02X", mfp[0], mfp[1], mfp[2], mfp[3]);
        lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -80, 50);

        lv_obj_t *qrCont = GuiCreateContainerWithParent(g_widgets.qrCont, 336, 336);
        lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 116);
        GuiAnimatingQRCodeInitWithCustomSize(qrCont, GuiGenerateUR, false, 336, 336, NULL);
        yOffset += 344;

        yOffset += 8;
        g_widgets.pubkey = GuiCreateNoticeLabel(g_widgets.qrCont, "");
        lv_obj_set_width(g_widgets.pubkey, 336);
        lv_obj_align(g_widgets.pubkey, LV_ALIGN_TOP_MID, 0, yOffset);
        return;
    }
#endif

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

    cont = GuiCreateContainerWithParent(parent, 408, 102 * g_btcPathNum);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    for (uint32_t i = 0; i < g_btcPathNum; i++) {
        label = GuiCreateTextLabel(cont, g_pathTypeList[i].title);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16 + 103 * i);
#ifndef BTC_ONLY
        snprintf_s(desc, BUFFER_SIZE_64, "%s (%s)", g_pathTypeList[i].subTitle, g_pathTypeList[i].path);
#else
        path = g_isTest ? g_btcTestNetPath[i] : g_pathTypeList[i].path;
        snprintf_s(desc, BUFFER_SIZE_64, "%s (%s)", g_pathTypeList[i].subTitle, path);
#endif
        label = GuiCreateNoticeLabel(cont, desc);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i != g_btcPathNum) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * (i + 1));
        }
        g_widgets.selectItems[i].checkBox = GuiCreateSelectPathCheckBox(cont);
        lv_obj_align(g_widgets.selectItems[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
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
        label = GuiCreateIllustrateLabel(egCont, "");
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 50 + 34 * i);
        lv_obj_set_width(label, 360);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_label_set_recolor(label, true);
        g_widgets.egs[i] = label;
    }
#ifdef BTC_ONLY
    if (g_isMultisig) {
        lv_obj_add_flag(egCont, LV_OBJ_FLAG_HIDDEN);
    }
#endif

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
    lv_obj_t *checkBox;

    checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < g_btcPathNum; i++) {
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

static void ConfirmHandler(lv_event_t *e)
{
    if (g_tmpSelectIndex != GetPathType()) {
        SetPathType(g_tmpSelectIndex);
        GuiGotoTileview(TILEVIEW_QRCODE);
    }
}

#ifdef BTC_ONLY
static ChainType ConvertChainType(ChainType chainType)
{
    switch (chainType) {
    case XPUB_TYPE_BTC_TAPROOT:
        return g_isTest ? XPUB_TYPE_BTC_TAPROOT_TEST : XPUB_TYPE_BTC_TAPROOT;
    case XPUB_TYPE_BTC_NATIVE_SEGWIT:
        return g_isTest ? XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST : XPUB_TYPE_BTC_NATIVE_SEGWIT;
    case XPUB_TYPE_BTC:
        return g_isTest ? XPUB_TYPE_BTC_TEST : XPUB_TYPE_BTC;
    case XPUB_TYPE_BTC_LEGACY:
        return g_isTest ? XPUB_TYPE_BTC_LEGACY_TEST : XPUB_TYPE_BTC_LEGACY;
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
    ChainType chainType = g_pathTypeList[pathType].pubkeyType;
#else
    ChainType chainType = ConvertChainType(g_pathTypeList[pathType].pubkeyType);
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
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH:
        head[0] = 'x';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST:
        head[0] = 't';
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
        return g_pathTypeList[pathType].title;
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
        snprintf_s(dest, maxLen, "%s (%s)", g_pathTypeList[pathType].subTitle, g_pathTypeList[pathType].path);
        break;
    default:
        printf("(GetPathTypeDesc) unsupported chain type: %d\r\n", chain);
    }
}
#else
static void GetPathTypeDesc(char *dest, uint16_t chain, uint8_t pathType, uint32_t maxLen)
{
    ASSERT(chain == CHAIN_BTC);
    const char *path = g_isTest ? g_btcTestNetPath[pathType] : g_pathTypeList[pathType].path;
    if (g_isMultisig) {
        strcpy_s(dest, maxLen, path);
    } else {
        snprintf_s(dest, maxLen, "%s (%s)", g_pathTypeList[pathType].subTitle, path);
    }
}
#endif

static void RefreshQrcode()
{
    uint8_t pathType = GetPathType();
    char pubkey[BUFFER_SIZE_128];
    GetExportPubkey(pubkey, g_chain, pathType, sizeof(pubkey));
    lv_label_set_text(g_widgets.pubkey, pubkey);
    lv_label_set_text(g_widgets.title, GetPathTypeTitle(g_chain, pathType));
    char desc[BUFFER_SIZE_32] = {0};
    GetPathTypeDesc(desc, g_chain, pathType, sizeof(desc));
    lv_label_set_text(g_widgets.desc, desc);
#ifdef BTC_ONLY
    if (g_isMultisig) {
        lv_obj_t *qrCont = GuiCreateContainerWithParent(g_widgets.qrCont, 336, 336);
        lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 116);
        GuiAnimatingQRCodeInitWithCustomSize(qrCont, GuiGenerateUR, false, 336, 336, NULL);
        return;
    }
#endif
    lv_qrcode_update(g_widgets.qrCode, pubkey, strnlen_s(pubkey, BUFFER_SIZE_128));
    lv_qrcode_update(g_widgets.qrCodeFullscreen, pubkey, strnlen_s(pubkey, BUFFER_SIZE_128));
    lv_obj_update_layout(g_widgets.pubkey);
    lv_obj_set_height(g_widgets.qrCont, lv_obj_get_height(g_widgets.pubkey) + 484);
}

static uint8_t GetPathType()
{
    int index = GetCurrentAccountIndex();
    int shift = g_isMultisig ? 4 : 0;
    uint8_t type = 0;
    switch (g_chain) {
    case CHAIN_BTC:
        type = 0xF & (g_btcPathType[index] >> shift);
        break;
    default:
        break;
    }
    return type;
}

static void SetPathType(uint8_t pathType)
{
    int index = GetCurrentAccountIndex();
    int shift = g_isMultisig ? 4 : 0;
    switch (g_chain) {
    case CHAIN_BTC:
        g_btcPathType[index] &= ~(0xF << shift);
        g_btcPathType[index] |= (pathType & 0x0F) << shift;
        break;

    default:
        break;
    }
}

#ifndef BTC_ONLY
static void ModelGetUtxoAddress(char *dest, uint8_t pathType, uint32_t index, uint32_t maxLen)
{
    char *xPub, hdPath[BUFFER_SIZE_128];
    xPub = GetCurrentAccountPublicKey(g_pathTypeList[pathType].pubkeyType);
    ASSERT(xPub);
    SimpleResponse_c_char *result;
    snprintf_s(hdPath, sizeof(hdPath), "%s/0/%u", g_pathTypeList[pathType].path, index);
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
    if (g_isMultisig) {
        return;
    }
    char *xPub, hdPath[128];
    const char *rootPath;
    ChainType chainType = ConvertChainType(g_pathTypeList[pathType].pubkeyType);
    rootPath = g_isTest ? g_btcTestNetPath[pathType] : g_pathTypeList[pathType].path;
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
#ifdef BTC_ONLY
    if (g_isMultisig) {
        return;
    }
#endif
    char eg[BUFFER_SIZE_64] = {};
    char prefix[8] = {0};
    char rest[BUFFER_SIZE_64] = {0};
    char addr[BUFFER_SIZE_128] = {0};
    char addrShot[BUFFER_SIZE_64] = {0};
    int8_t prefixLen = (g_pathTypeList[index].pubkeyType == XPUB_TYPE_BTC_NATIVE_SEGWIT || g_pathTypeList[index].pubkeyType == XPUB_TYPE_BTC_TAPROOT) ? 4 : 1;
    for (uint8_t i = 0; i < 2; i++) {
        memset_s(addrShot, BUFFER_SIZE_64, 0, BUFFER_SIZE_64);
        ModelGetUtxoAddress(addr, index, i, sizeof(addr));
        CutAndFormatString(addrShot, sizeof(addrShot), addr, 24);
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
    for (uint8_t i = 0; i < g_btcPathNum; i++) {
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
