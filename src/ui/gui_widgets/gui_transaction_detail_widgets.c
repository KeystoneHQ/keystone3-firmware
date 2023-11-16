#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_transaction_detail_widgets.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_analyze.h"
#include "gui_button.h"
#include "gui_qr_code.h"
#include "secret_cache.h"
#include "qrdecode_task.h"
#include "gui_chain.h"
#include "assert.h"
#include "gui_web_auth_widgets.h"
#include "gui_qr_hintbox.h"
#include "motor_manager.h"
#include "gui_lock_widgets.h"
#include "screen_manager.h"
#include "fingerprint_process.h"
#include "gui_fullscreen_mode.h"
#include "gui_keyboard_hintbox.h"
#include "gui_page.h"
#include "account_manager.h"
#include "gui_pending_hintbox.h"
#include "eapdu_services/service_resolve_ur.h"
#ifndef COMPILE_SIMULATOR
#include "keystore.h"

#else
#define FP_SUCCESS_CODE 0
#define RECOGNIZE_UNLOCK 0
#define RECOGNIZE_OPEN_SIGN 1
#define RECOGNIZE_SIGN 2

#define NO_ENCRYPTION 0
#define AES_KEY_ENCRYPTION 1
#define RESET_AES_KEY_ENCRYPTION 2
#define FINGERPRINT_EN_SING_ERR_TIMES (5)
#define FINGERPRINT_RESPONSE_MSG_LEN (23)
#define FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT (0xFF)
#define FINGERPRINT_SING_ERR_TIMES (3)
#define FINGERPRINT_SING_DISABLE_ERR_TIMES (15)
#endif

#define QRCODE_CONFIRM_SIGN_PROCESS 66
#define FINGER_SIGN_MAX_COUNT 5

static ViewType g_viewType;
static uint8_t g_chainType = CHAIN_BUTT;
static PageWidget_t *g_pageWidget;
static KeyboardWidget_t *g_keyboardWidget = NULL;

static lv_obj_t *g_fingerSingContainer = NULL;
static lv_obj_t *g_fpErrorImg = NULL;
static lv_obj_t *g_fpErrorLabel = NULL;
static uint32_t g_fingerSignCount = FINGER_SIGN_MAX_COUNT;
static uint32_t g_fingerSignErrCount = 0;
static lv_timer_t *g_fpRecognizeTimer;

typedef enum {
    TRANSACTION_MODE_QR_CODE = 0,
    TRANSACTION_MODE_USB,
} TransactionMode;

static void GuiTransactionDetailNavBarInit();
static void CheckSliderProcessHandler(lv_event_t *e);
static void SignByPasswordCb(bool cancel);
static void SignByPasswordCbHandler(lv_event_t *e);
static void CloseContHandler(lv_event_t *e);
static void SignByFinger(void);
static void RecognizeFailHandler(lv_timer_t *timer);
static TransactionMode GetCurrentTransactionMode(void);
static void TransactionGoToHomeViewHandler(lv_event_t *e);

static TransactionMode GetCurrentTransactionMode(void)
{
    uint16_t requestID = GetCurrentUSParsingRequestID();
    if (requestID != 0)
    {
        return TRANSACTION_MODE_USB;
    }
    return TRANSACTION_MODE_QR_CODE;
}

static void TransactionGoToHomeViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (GetCurrentTransactionMode() == TRANSACTION_MODE_USB)
        {
            const char *data = "UR parsing rejected";
            HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_REJECTED);
        }
        CloseQRTimer();
        GuiCloseToTargetView(&g_homeView);
    }
}

void GuiTransactionDetailInit(uint8_t viewType)
{
    g_viewType = viewType;
    g_chainType = ViewTypeToChainTypeSwitch(g_viewType);
    g_pageWidget = CreatePageWidget();
    GuiTransactionDetailNavBarInit();
    ParseTransaction(g_viewType);
    g_fingerSignCount = 0;
    GuiCreateConfirmSlider(g_pageWidget->contentZone, CheckSliderProcessHandler);
    GuiPendingHintBoxMoveToTargetParent(lv_scr_act());
}



void GuiTransactionDetailDeInit()
{
    // for learn more hintbox in eth contract data block;
    if (GuiQRHintBoxIsActive())
    {
        GuiQRHintBoxRemove();
    }
    GUI_DEL_OBJ(g_fingerSingContainer)
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    GuiTemplateClosePage();
}



void GuiTransactionDetailRefresh()
{

}

void GuiTransactionDetailParseSuccess(void *param)
{
    SetParseTransactionResult(param);
    GuiTemplateReload(g_pageWidget->contentZone, g_viewType);
}

void GuiTransactionDetailVerifyPasswordSuccess(void)
{
    GUI_DEL_OBJ(g_fingerSingContainer)
    GuiDeleteKeyboardWidget(g_keyboardWidget);

    if (GetCurrentTransactionMode() == TRANSACTION_MODE_USB)
    {
        GenerateUR func = GetSingleUrGenerator(g_viewType);
        if (func == NULL)
        {
            return;
        }
        UREncodeResult *urResult = func();
        if (urResult->error_code == 0)
        {
            HandleURResultViaUSBFunc(urResult->data, strlen(urResult->data), GetCurrentUSParsingRequestID(), RSP_SUCCESS_CODE);
        }
        else if (urResult->error_code == 2)
        {
            const char *data = "Mismatched wallet, please switch to another wallet and try again";
            HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_MISMATCHED_WALLET);
        }
        else
        {
            HandleURResultViaUSBFunc(urResult->error_message, strlen(urResult->error_message), GetCurrentUSParsingRequestID(), PRS_PARSING_ERROR);
        }
        return;
    }

    GuiFrameOpenViewWithParam(&g_transactionSignatureView, &g_viewType, sizeof(g_viewType));
}

void GuiSignVerifyPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void GuiSignDealFingerRecognize(void *param)
{
    uint8_t errCode = *(uint8_t *)param;
    static uint16_t passCodeType = ENTER_PASSCODE_VERIFY_PASSWORD;
    if (g_fingerSingContainer == NULL)
    {
        return;
    }
    if (errCode == FP_SUCCESS_CODE)
    {
        lv_img_set_src(g_fpErrorImg, &imgYellowFinger);
        GuiModelVerifyAmountPassWord(&passCodeType);
        g_fingerSignErrCount = 0;
    }
    else
    {
        g_fingerSignErrCount++;
        g_fingerSignCount++;
        if (g_fpErrorLabel != NULL && lv_obj_has_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);
        }
        lv_img_set_src(g_fpErrorImg, &imgRedFinger);
        printf("GuiSignDealFingerRecognize err message is %s\n", GetFpErrorMessage(errCode));
        printf("g_fingerSingCount is %d\n", g_fingerSignCount);
        if (g_fingerSignCount < FINGERPRINT_SING_ERR_TIMES)
        {
            FpRecognize(RECOGNIZE_SIGN);
            g_fpRecognizeTimer = lv_timer_create(RecognizeFailHandler, 1000, NULL);
        }
        else
        {
            SignByPasswordCb(false);
        }
        printf("g_fingerSignErrCount.... = %d\n", g_fingerSignErrCount);
        if (g_fingerSignErrCount >= FINGERPRINT_SING_DISABLE_ERR_TIMES)
        {
            for (int i = 0; i < 3; i++)
            {
                UpdateFingerSignFlag(i, false);
            }
        }
    }
}

void GuiClearQrcodeSignCnt(void)
{
    g_fingerSignErrCount = 0;
}

static void GuiTransactionDetailNavBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, TransactionGoToHomeViewHandler, NULL);
    if (IsMessageType(g_viewType)) {
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("transaction_parse_confirm_message"));
    } else {
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, NULL);
    }
}

static void CheckSliderProcessHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED)
    {
        int32_t value = lv_slider_get_value(lv_event_get_target(e));
        if (value >= QRCODE_CONFIRM_SIGN_PROCESS)
        {
            if ((GetCurrentAccountIndex() < 3) && GetFingerSignFlag() && g_fingerSignCount < 3)
            {
                SignByFinger();
            }
            else
            {
                SignByPasswordCb(false);
            }
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_OFF);
        }
        else
        {
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_ON);
        }
    }
}

static void SignByPasswordCb(bool cancel)
{
    GUI_DEL_OBJ(g_fingerSingContainer)
    if (cancel)
    {
        FpCancelCurOperate();
    }
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
}

static void SignByPasswordCbHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        SignByPasswordCb(true);
    }
}

static void CloseContHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        GUI_DEL_OBJ(g_fingerSingContainer)
    }
}

static void SignByFinger(void)
{
    GUI_DEL_OBJ(g_fingerSingContainer)

    g_fingerSingContainer = GuiCreateHintBox(lv_scr_act(), 480, 428, true);
    lv_obj_t *cont = g_fingerSingContainer;
    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("scan_qr_code_sign_fingerprint_verify_fingerprint"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 402);

    lv_obj_t *img = GuiCreateImg(cont, &imgClose);
    GuiButton_t table[2] = {
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {14, 14},
        }};
    lv_obj_t *button = GuiCreateButton(cont, 64, 64, table, 1, CloseContHandler, cont);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 384, 394);

    g_fpErrorImg = GuiCreateImg(cont, &imgYellowFinger);
    lv_obj_align(g_fpErrorImg, LV_ALIGN_BOTTOM_MID, 0, -178);

    lv_obj_t *arc = GuiCreateArc(cont);
    lv_obj_set_style_arc_opa(arc, LV_OPA_10, LV_PART_MAIN);
    lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, -154);

    g_fpErrorLabel = GuiCreateLabel(cont, _("scan_qr_code_sign_unsigned_content_fingerprint_failed_desc"));
    lv_obj_set_style_text_color(g_fpErrorLabel, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(g_fpErrorLabel, LV_ALIGN_BOTTOM_MID, 0, -100);
    lv_obj_add_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);

    label = GuiCreateNoticeLabel(cont, _("scan_qr_code_sign_fingerprint_enter_passcode"));
    img = GuiCreateImg(cont, &imgLockedLock);
    table[0].obj = label;
    table[0].align = LV_ALIGN_DEFAULT;
    table[0].position.x = 40;
    table[0].position.y = 3;
    table[1].obj = img;
    table[1].align = LV_ALIGN_DEFAULT;
    table[1].position.x = 8;
    table[1].position.y = 6;

    button = GuiCreateButton(cont, 192, 36, table, NUMBER_OF_ARRAYS(table), SignByPasswordCbHandler, cont);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -27);
    FpRecognize(RECOGNIZE_SIGN);
}

static void RecognizeFailHandler(lv_timer_t *timer)
{
    if (g_fingerSingContainer != NULL)
    {
        lv_img_set_src(g_fpErrorImg, &imgYellowFinger);
        lv_obj_add_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);
    }
    lv_timer_del(timer);
    g_fpRecognizeTimer = NULL;
}