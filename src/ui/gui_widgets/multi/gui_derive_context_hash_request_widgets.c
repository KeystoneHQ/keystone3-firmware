#include "gui.h"
#include "gui_page.h"
#include "librust_c.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_chain.h"
#include "secret_cache.h"
#include "account_manager.h"
#include "account_public_info.h"
#include "gui_keyboard_hintbox.h"
#include "gui_lock_widgets.h"
#include "gui_animating_qrcode.h"
#include "gui_attention_hintbox.h"
#include "gui_derive_context_hash_request_widgets.h"

typedef struct {
    uint8_t currentTile;
    PageWidget_t *pageWidget;
    lv_obj_t *tileView;
    lv_obj_t *qrCode;
} DeriveContextHashWidget_t;

typedef enum {
    TILE_APPROVE,
    TILE_QRCODE,

    TILE_BUTT,
} PAGE_TILE;

// Slider value (0-100) at which the confirm action triggers, matching the sign-tx UI.
#define DCH_CONFIRM_SLIDE_PROCESS 66

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static bool g_isMulti = false;
static void *g_data = NULL;
static DeriveContextHashWidget_t g_widget;
static DeriveContextHashCallData *g_callData = NULL;
static Response_DeriveContextHashCallData *g_response = NULL;
static char *g_address = NULL;
static lv_obj_t *g_approveTile = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;

static void FreeDeriveContextHashMemory(void);
static void ModelParse(void);
static bool DeriveConnectedAddress(void);
static UREncodeResult *ModelGenerateSyncUR(void);
static void GuiCreateApproveWidget(lv_obj_t *parent);
static void GuiCreateQRCodeWidget(lv_obj_t *parent);
static void ConfirmSliderHandler(lv_event_t *e);
static void CloseInvalidRequestHandler(lv_event_t *e);
static void OnReturnHandler(lv_event_t *e);
static void NormalizeDisplayKeyPath(char *out, uint32_t outSize, const char *keyPath);
static void GuiShowKeyBoardDialog(lv_obj_t *parent);
static void NextTile(void);
static void PrevTile(void);

void GuiSetDeriveContextHashRequestData(void *urResult, void *multiResult, bool is_multi)
{
    FreeDeriveContextHashMemory();
    g_urResult = urResult;
    g_urMultiResult = multiResult;
    g_isMulti = is_multi;
    g_data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
}

static void FreeDeriveContextHashMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    if (g_response != NULL) {
        free_Response_DeriveContextHashCallData(g_response);
        g_response = NULL;
    }
    if (g_address != NULL) {
        SRAM_FREE(g_address);
        g_address = NULL;
    }
}

static void ModelParse(void)
{
    g_response = parse_derive_context_hash(g_data);
    g_callData = g_response->data;
}

// Derive the connected key's address (seedless, from the cached account xpub) so the
// user can confirm it on the approval screen.
static bool DeriveConnectedAddress(void)
{
    if (g_callData == NULL || g_callData->key_path == NULL) {
        return false;
    }

    const char *keyPath = g_callData->key_path;
    if (strncmp(keyPath, "m/", 2) == 0 || strncmp(keyPath, "M/", 2) == 0) {
        keyPath += 2;
    }
    int purpose = atoi(keyPath);
    ChainType xpubType;
    switch (purpose) {
    case 44:
        xpubType = XPUB_TYPE_BTC_LEGACY;
        break;
    case 49:
        xpubType = XPUB_TYPE_BTC;
        break;
    case 84:
        xpubType = XPUB_TYPE_BTC_NATIVE_SEGWIT;
        break;
    case 86:
        xpubType = XPUB_TYPE_BTC_TAPROOT;
        break;
    default:
        return false;
    }
    char *xpub = GetCurrentAccountPublicKey(xpubType);
    if (xpub == NULL) {
        return false;
    }
    char hdPath[BUFFER_SIZE_64] = {0};
    NormalizeDisplayKeyPath(hdPath, sizeof(hdPath), g_callData->key_path);
    if (hdPath[0] == '\0') {
        return false;
    }
    SimpleResponse_c_char *result = btcoin_get_address_with_network(hdPath, xpub, g_callData->network);
    if (result->error_code == 0 && result->data != NULL) {
        g_address = SRAM_MALLOC(strnlen_s(result->data, BUFFER_SIZE_128) + 1);
        strcpy(g_address, result->data);
        free_simple_response_c_char(result);
        return true;
    }
    free_simple_response_c_char(result);
    return false;
}

void GuiDeriveContextHashRequestInit(bool isUsb)
{
    GUI_PAGE_DEL(g_widget.pageWidget);
    g_widget.pageWidget = CreatePageWidget();
    SetNavBarLeftBtn(g_widget.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_widget.pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Derive Context Hash"));

    ModelParse();
    // Rust validated the request (appName allow-list, network, context, key path).
    // On failure `g_callData` is NULL, so show the error and skip the confirm UI
    // instead of dereferencing it. The nav-bar back button returns to the scanner.
    if (g_response->error_code != 0 || g_callData == NULL) {
        char *message = g_response->error_message != NULL
                        ? g_response->error_message
                        : (char *)_("Invalid derive-context-hash request");
        GuiCreateHardwareCallInvaildParamHintboxWithHandler(
            (char *)_("Invalid Request"), message, CloseInvalidRequestHandler);
        return;
    }
    if (!DeriveConnectedAddress()) {
        GuiCreateHardwareCallInvaildParamHintboxWithHandler(
            (char *)_("Invalid Request"), (char *)_("Invalid derive-context-hash key path"),
            CloseInvalidRequestHandler);
        return;
    }

    lv_obj_t *tileView = GuiCreateTileView(g_widget.pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, TILE_APPROVE, 0, LV_DIR_HOR);
    GuiCreateApproveWidget(tile);

    tile = lv_tileview_add_tile(tileView, TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQRCodeWidget(tile);

    g_widget.tileView = tileView;
    g_widget.currentTile = TILE_APPROVE;
    lv_obj_set_tile_id(g_widget.tileView, g_widget.currentTile, 0, LV_ANIM_OFF);
}

void GuiDeriveContextHashRequestDeInit(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GuiCloseAttentionHintbox();
    GUI_PAGE_DEL(g_widget.pageWidget);
    GuiAnimatingQRCodeDestroyTimer();
    FreeDeriveContextHashMemory();
    g_approveTile = NULL;
    SetPageLockScreen(true);
}

void GuiDeriveContextHashRequestRefresh(void)
{
    GuiAnimatingQRCodeControl(false);
}

static void CreateInfoRow(lv_obj_t *parent, const char *title, const char *value)
{
    GuiCreateNoticeLabel(parent, title);
    lv_obj_t *valueLabel = GuiCreateIllustrateLabel(parent, value == NULL ? "" : value);
    lv_obj_set_width(valueLabel, lv_pct(100));
    lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
}

static void NormalizeDisplayKeyPath(char *out, uint32_t outSize, const char *keyPath)
{
    if (out == NULL || outSize == 0) {
        return;
    }
    out[0] = '\0';
    if (keyPath == NULL) {
        return;
    }
    if (strncmp(keyPath, "m/", 2) == 0 || strncmp(keyPath, "M/", 2) == 0) {
        snprintf_s(out, outSize, "%s", keyPath);
        out[0] = 'M';
    } else {
        snprintf_s(out, outSize, "M/%s", keyPath);
    }
}

static void GuiCreateApproveWidget(lv_obj_t *parent)
{
    g_approveTile = parent;
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("Derive Context Hash Request"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 8);

    // Outer fixed-height card = the visible scroll window.
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 480);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_12, LV_PART_MAIN);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);

    // Inner content grows with the fields (flex column, height = content). When it is
    // taller than the card, the card scrolls vertically.
    lv_obj_t *inner = lv_obj_create(cont);
    lv_obj_set_width(inner, lv_pct(100));
    lv_obj_set_height(inner, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(inner, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(inner, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(inner, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(inner, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_row(inner, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(inner, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(inner, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(inner, LV_OBJ_FLAG_SCROLLABLE);

    char path[BUFFER_SIZE_64] = {0};
    NormalizeDisplayKeyPath(path, sizeof(path), g_callData->key_path);
    CreateInfoRow(inner, _("App Name"), g_callData->app_name);
    CreateInfoRow(inner, _("From"), g_callData->origin);
    CreateInfoRow(inner, _("Network"), g_callData->network);
    CreateInfoRow(inner, _("Path"), path);
    if (g_address != NULL) {
        CreateInfoRow(inner, _("Address"), g_address);
    }
    CreateInfoRow(inner, _("Context"), g_callData->context);

    // Reuse the sign-tx "slide to confirm" component for a consistent approval UX.
    // (Reject is handled by the nav-bar back button.)
    GuiCreateConfirmSlider(parent, ConfirmSliderHandler);
}

static void GuiCreateQRCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_scan"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 408);
    lv_obj_add_flag(qrCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(qrCont, 336, 336);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_color(qrBgCont, WHITE_COLOR, LV_PART_MAIN);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(qrBgCont, 294, 294);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);
    g_widget.qrCode = qrcode;
}

static void GuiShowKeyBoardDialog(lv_obj_t *parent)
{
    g_keyboardWidget = GuiCreateKeyboardWidget(parent);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_HARDWARE_CALL_DERIVE_PUBKEY;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void ConfirmSliderHandler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) {
        return;
    }
    lv_obj_t *slider = lv_event_get_target(e);
    if (lv_slider_get_value(slider) >= DCH_CONFIRM_SLIDE_PROCESS) {
        lv_slider_set_value(slider, 0, LV_ANIM_OFF);
        GuiShowKeyBoardDialog(g_approveTile);
    } else {
        lv_slider_set_value(slider, 0, LV_ANIM_ON);
    }
}

static void CloseInvalidRequestHandler(lv_event_t *e)
{
    GuiCloseAttentionHintbox();
    CloseCurrentViewHandler(e);
}

static void OnReturnHandler(lv_event_t *e)
{
    PrevTile();
}

static UREncodeResult *ModelGenerateSyncUR(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    uint8_t seed[64] = {0};
    char *password = SecretCacheGetPassword();
    MnemonicType mnemonicType = GetMnemonicType();
    int seedLen = (mnemonicType == MNEMONIC_TYPE_SLIP39) ? GetCurrentAccountEntropyLen() : sizeof(seed);
    GetAccountSeed(GetCurrentAccountIndex(), seed, password);
    UREncodeResult *urResult = generate_derive_context_hash_ur(g_data, seed, seedLen, g_address);
    memset(seed, 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    return urResult;
}

void DeriveContextHashHiddenKeyboardAndShowAnimateQR(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GuiAnimatingQRCodeInit(g_widget.qrCode, ModelGenerateSyncUR, true);
    NextTile();
}

void GuiDeriveContextHashWidgetHandleURGenerate(char *data, uint16_t len)
{
    GuiAnimantingQRCodeFirstUpdate(data, len);
}

void GuiDeriveContextHashWidgetHandleURUpdate(char *data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
}

void GuiDeriveContextHashPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void GuiDeriveContextHashUsbPullout(void)
{
    // USB transport is not wired for derive-context-hash yet (QR only).
}

static void NextTile(void)
{
    g_widget.currentTile++;
    if (g_widget.currentTile == TILE_QRCODE) {
        SetNavBarLeftBtn(g_widget.pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
    }
    lv_obj_set_tile_id(g_widget.tileView, g_widget.currentTile, 0, LV_ANIM_OFF);
}

static void PrevTile(void)
{
    g_widget.currentTile--;
    if (g_widget.currentTile == TILE_APPROVE) {
        SetNavBarLeftBtn(g_widget.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        GuiAnimatingQRCodeDestroyTimer();
    }
    lv_obj_set_tile_id(g_widget.tileView, g_widget.currentTile, 0, LV_ANIM_OFF);
}
