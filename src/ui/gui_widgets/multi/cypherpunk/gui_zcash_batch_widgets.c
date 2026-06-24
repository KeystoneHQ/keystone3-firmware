#include "gui_zcash_batch_widgets.h"
#include "account_manager.h"
#include "gui_button.h"
#include "gui_chain.h"
#include "gui_chain_components.h"
#include "gui_hintbox.h"
#include "gui_keyboard_hintbox.h"
#include "gui_lock_widgets.h"
#include "gui_model.h"
#include "gui_page.h"
#include "gui_views.h"
#include "gui_zcash.h"
#include "keystore.h"
#include "screen_manager.h"
#include "general/eapdu_services/service_resolve_ur.h"
#include "user_memory.h"

#define QRCODE_CONFIRM_SIGN_PROCESS 66
#define USB_REQUEST_IDLE 0xFFFF
#define ZCASH_BATCH_CONTENT_HEIGHT 656
#define ZCASH_BATCH_SIGN_SLIDER_HEIGHT 114
#define ZCASH_BATCH_BOTTOM_BTN_HEIGHT 90
#define ZCASH_BATCH_BOTTOM_BTN_MARGIN 12

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static bool g_isMulti = false;

static uint32_t g_currentTxIndex = 0;
static uint32_t g_txCount = 0;

static TransactionParseResult_DisplayZcashBatch *g_parseResult = NULL;
static DisplayZcashBatch *g_displayZcashBatch = NULL;
static DisplayPczt *g_currentTransaction = NULL;

static PageWidget_t *g_pageWidget = NULL;
static lv_obj_t *g_cont = NULL;
static lv_obj_t *g_txContainer = NULL;
static lv_obj_t *g_bottomBtnContainer = NULL;
static lv_obj_t *g_signSlider = NULL;
static lv_obj_t *g_parseErrorHintBox = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;

static void *GuiParseZcashBatchData(void);
static void CheckSliderProcessHandler(lv_event_t *e);
static void GuiRenderCurrentTransaction(bool showSignSlider);
static void GuiRenderBottomBtn(lv_obj_t *parent, bool showSignSlider);
static void HandleClickPreviousBtn(lv_event_t *e);
static void HandleClickNextBtn(lv_event_t *e);
static void GuiReturnHome(void);
static void CloseParseErrorHandler(lv_event_t *e);
static UREncodeResult *SignZcashBatchInternal(void *data, bool unlimited);
static bool IsZcashBatchUsbMode(void);
static void RejectZcashBatchUsbRequest(void);
static void RespondZcashBatchUsbParseError(const char *errorMessage);

static void ClearPageData(void)
{
    g_currentTxIndex = 0;
    g_txCount = 0;
    g_currentTransaction = NULL;
    g_displayZcashBatch = NULL;

    if (g_parseResult != NULL) {
        free_TransactionParseResult_DisplayZcashBatch(g_parseResult);
        g_parseResult = NULL;
    }

    if (g_isMulti) {
        CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    } else {
        CHECK_FREE_UR_RESULT(g_urResult, false);
    }
}

void GuiSetZcashBatchUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

UREncodeResult *GuiGetZcashBatchSignQrCodeData(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    return SignZcashBatchInternal(data, false);
}

UREncodeResult *GuiGetZcashBatchSignUrDataUnlimited(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    return SignZcashBatchInternal(data, true);
}

static bool IsZcashBatchUsbMode(void)
{
    return GetCurrentUSParsingRequestID() != USB_REQUEST_IDLE;
}

static void RejectZcashBatchUsbRequest(void)
{
    if (!IsZcashBatchUsbMode()) {
        return;
    }

    const char *data = "UR parsing rejected";
    HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_REJECTED);
}

static void RespondZcashBatchUsbParseError(const char *errorMessage)
{
    if (!IsZcashBatchUsbMode()) {
        return;
    }

    const char *data = (errorMessage != NULL && strlen(errorMessage) > 0)
                           ? errorMessage
                           : "UR parsing failed";
    HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_ERROR);
}

static UREncodeResult *SignZcashBatchInternal(void *data, bool unlimited)
{
    return GuiSignZcashCypherpunkWithSeed(
               data,
               unlimited,
               sign_zcash_batch_tx_cypherpunk,
               sign_zcash_batch_tx_cypherpunk_unlimited);
}

#ifdef CYPHERPUNK_VERSION
PtrT_TransactionCheckResult GuiGetZcashBatchCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t sfp[32] = {0};
    uint32_t zcashAccountIndex = 0;
    uint8_t accountNum = 0;
    char ufvk[ZCASH_UFVK_MAX_LEN + 1] = {0};

    GetExistAccountNum(&accountNum);
    if (accountNum <= 0) {
        return check_zcash_batch_tx_cypherpunk(data, ufvk, sfp, zcashAccountIndex, true);
    }

    GetZcashSFP(GetCurrentAccountIndex(), sfp);
    GetZcashUFVK(GetCurrentAccountIndex(), ufvk);
    return check_zcash_batch_tx_cypherpunk(
               data,
               ufvk,
               sfp,
               zcashAccountIndex,
               !IsZcashSupportedForCurrentMnemonic());
}
#endif

void GuiZcashBatchWidgetsVerifyPasswordSuccess(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    if (IsZcashBatchUsbMode()) {
        UREncodeResult *urResult = GuiGetZcashBatchSignUrDataUnlimited();
        if (urResult != NULL && urResult->error_code == 0) {
            if (urResult->is_multi_part) {
                const char *errorMessage = "Signing result is too large for USB transport";
                HandleURResultViaUSBFunc(errorMessage, strlen(errorMessage), GetCurrentUSParsingRequestID(), PRS_PARSING_ERROR);
            } else {
                HandleURResultViaUSBFunc(urResult->data, strlen(urResult->data), GetCurrentUSParsingRequestID(), RSP_SUCCESS_CODE);
            }
        } else {
            const char *errorMessage = (urResult != NULL && urResult->error_message != NULL) ? urResult->error_message : "Signing failed";
            HandleURResultViaUSBFunc(errorMessage, strlen(errorMessage), GetCurrentUSParsingRequestID(), PRS_PARSING_ERROR);
        }
        if (urResult != NULL) {
            free_ur_encode_result(urResult);
        }
        return;
    }
    uint8_t viewType = ZcashBatchTx;
    GuiFrameOpenViewWithParam(&g_transactionSignatureView, &viewType, sizeof(viewType));
}

void GuiZcashBatchWidgetsSignVerifyPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    if (passwordVerifyResult->errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX &&
            IsZcashBatchUsbMode()) {
        const char *data = "Please try again after unlocking";
        HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_VERIFY_PASSWORD_ERROR);
    }
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void GuiZcashBatchWidgetsUsbPullout(void)
{
    if (!IsZcashBatchUsbMode()) {
        return;
    }

    GuiDeleteKeyboardWidget(g_keyboardWidget);
    ClearUSBRequestId();
    GuiReturnHome();
}

static void SignByPasswordCb(bool cancel)
{
    (void)cancel;
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_SIGN_TRANSACTION_WITH_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void CheckSliderProcessHandler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) {
        return;
    }

    int32_t value = lv_slider_get_value(lv_event_get_target(e));
    if (value >= QRCODE_CONFIRM_SIGN_PROCESS) {
        SignByPasswordCb(false);
        lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_OFF);
    } else {
        lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_ON);
    }
}

static void HandleClickPreviousBtn(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED && g_currentTxIndex > 0) {
        g_currentTxIndex--;
        GuiZcashBatchWidgetsRefresh();
    }
}

static void HandleClickNextBtn(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED && g_currentTxIndex < g_txCount - 1) {
        g_currentTxIndex++;
        GuiZcashBatchWidgetsRefresh();
    }
}

static void GuiReturnHome(void)
{
    GuiCloseToTargetView(&g_homeView);
}

static void CloseParseErrorHandler(lv_event_t *e)
{
    lv_obj_del(lv_event_get_user_data(e));
    g_parseErrorHintBox = NULL;
    GuiReturnHome();
}

static lv_obj_t *GuiCreateZcashBatchParseErrorWindow(const char *errorMessage)
{
    const char *descText = (errorMessage != NULL && strlen(errorMessage) > 0)
                               ? errorMessage
                               : _("scan_qr_code_error_invalid_qrcode_desc");
    lv_obj_t *cont = GuiCreateConfirmHintBox(
        &imgFailed,
        _("scan_qr_code_error_invalid_qrcode"),
        descText,
        NULL,
        _("OK"),
        WHITE_COLOR_OPA20);
    lv_obj_add_event_cb(GuiGetHintBoxRightBtn(cont), CloseParseErrorHandler, LV_EVENT_CLICKED, cont);
    return cont;
}

static void OnReturnHandler(lv_event_t *e)
{
    RejectZcashBatchUsbRequest();
    GuiReturnHome();
}

static void ZcashBatchNavBarInit(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
}

static void ZcashBatchNavBarRefresh(void)
{
    char text[BUFFER_SIZE_128] = {0};
    if (g_txCount > 1) {
        snprintf_s(
            text,
            sizeof(text),
            "%s (%d/%d)",
            _("confirm_transaction"),
            (int)(g_currentTxIndex + 1),
            (int)g_txCount);
    } else {
        snprintf_s(text, sizeof(text), "%s", _("confirm_transaction"));
    }
    SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_ZCASH, text);

    if (g_currentTxIndex == 0) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
    } else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, HandleClickPreviousBtn, NULL);
    }
}

static void GuiRenderBottomBtn(lv_obj_t *parent, bool showSignSlider)
{
    if (showSignSlider) {
        GUI_DEL_OBJ(g_bottomBtnContainer)
        if (g_signSlider == NULL) {
            g_signSlider = GuiCreateConfirmSlider(parent, CheckSliderProcessHandler);
        }
        return;
    }

    GUI_DEL_OBJ(g_signSlider)
    if (g_bottomBtnContainer != NULL) {
        return;
    }

    g_bottomBtnContainer = GuiCreateContainerWithParent(parent, 480, ZCASH_BATCH_BOTTOM_BTN_HEIGHT);
    lv_obj_align(g_bottomBtnContainer, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t *leftBtn = GuiCreateTextBtn(g_bottomBtnContainer, _("Previous"));
    lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 36, -ZCASH_BATCH_BOTTOM_BTN_MARGIN);
    lv_obj_set_size(leftBtn, 192, 66);
    lv_obj_set_style_bg_color(leftBtn, DARK_GRAY_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(leftBtn, HandleClickPreviousBtn, LV_EVENT_CLICKED, NULL);

    lv_obj_t *rightBtn = GuiCreateTextBtn(g_bottomBtnContainer, _("Next"));
    lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -ZCASH_BATCH_BOTTOM_BTN_MARGIN);
    lv_obj_set_size(rightBtn, 192, 66);
    lv_obj_set_style_bg_color(rightBtn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(rightBtn, HandleClickNextBtn, LV_EVENT_CLICKED, NULL);
}

static void GuiRenderCurrentTransaction(bool showSignSlider)
{
    GUI_DEL_OBJ(g_txContainer)

    uint16_t txHeight = showSignSlider
                        ? ZCASH_BATCH_CONTENT_HEIGHT - ZCASH_BATCH_SIGN_SLIDER_HEIGHT
                        : ZCASH_BATCH_CONTENT_HEIGHT - ZCASH_BATCH_BOTTOM_BTN_HEIGHT;

    g_txContainer = GuiCreateContainerWithParent(g_cont, 408, txHeight);
    lv_obj_align(g_txContainer, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(g_txContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(g_txContainer, NULL, LV_PART_SCROLLBAR);

    GuiZcashOverviewWithDataAndHeight(g_txContainer, g_currentTransaction, txHeight);
    GuiRenderBottomBtn(g_cont, showSignSlider);
}

void GuiZcashBatchWidgetsRefresh(void)
{
    if (g_parseResult == NULL || g_parseResult->error_code != 0) {
        return;
    }

    ZcashBatchNavBarRefresh();
    g_currentTransaction = &g_displayZcashBatch->txs->data[g_currentTxIndex];
    GuiRenderCurrentTransaction(g_currentTxIndex == g_txCount - 1);
}

static void *GuiParseZcashBatchData(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t sfp[32];
    uint32_t zcashAccountIndex = 0;
    GetZcashSFP(GetCurrentAccountIndex(), sfp);

    char ufvk[ZCASH_UFVK_MAX_LEN + 1] = {0};
    GetZcashUFVK(GetCurrentAccountIndex(), ufvk);
    g_parseResult = parse_zcash_batch_tx_cypherpunk(
        data,
        ufvk,
        sfp,
        zcashAccountIndex,
        !IsZcashSupportedForCurrentMnemonic());

    return g_parseResult;
}

void GuiZcashBatchWidgetsTransactionParseSuccess(void)
{
    g_displayZcashBatch = g_parseResult->data;
    g_txCount = g_displayZcashBatch->txs->size;
    g_currentTxIndex = 0;
    GuiZcashBatchWidgetsRefresh();
}

void GuiZcashBatchWidgetsTransactionParseFail(void)
{
    printf("GuiZcashBatchWidgetsTransactionParseFail\n");
    if (g_parseResult != NULL) {
        printf("error: %s\n", g_parseResult->error_message);
        if (IsZcashBatchUsbMode()) {
            RespondZcashBatchUsbParseError(g_parseResult->error_message);
            return;
        }
        g_parseErrorHintBox = GuiCreateZcashBatchParseErrorWindow(g_parseResult->error_message);
        return;
    }
    if (IsZcashBatchUsbMode()) {
        RespondZcashBatchUsbParseError(NULL);
        return;
    }
    g_parseErrorHintBox = GuiCreateErrorCodeWindow(ERR_INVALID_QRCODE, &g_parseErrorHintBox, GuiReturnHome);
}

void GuiZcashBatchWidgetsInit(void)
{
    g_pageWidget = CreatePageWidget();
    g_cont = g_pageWidget->contentZone;
    g_txContainer = NULL;
    g_bottomBtnContainer = NULL;
    g_signSlider = NULL;
    g_parseErrorHintBox = NULL;
    g_keyboardWidget = NULL;

    lv_obj_add_flag(g_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(g_cont, NULL, LV_PART_SCROLLBAR);

    ZcashBatchNavBarInit();
    GuiEmitSignal(SIG_SHOW_TRANSACTION_LOADING, NULL, 0);
    GuiModelParseTransaction(GuiParseZcashBatchData);
}

void GuiZcashBatchWidgetsDeInit(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GUI_PAGE_DEL(g_pageWidget);
    ClearPageData();
}
