#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_qrcode_widgets.h"
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
#ifndef COMPILE_SIMULATOR
#include "keystore.h"

#else
#define FP_SUCCESS_CODE             0
#define RECOGNIZE_UNLOCK            0
#define RECOGNIZE_OPEN_SIGN         1
#define RECOGNIZE_SIGN              2

#define NO_ENCRYPTION 0
#define AES_KEY_ENCRYPTION 1
#define RESET_AES_KEY_ENCRYPTION 2
#define FINGERPRINT_EN_SING_ERR_TIMES           (5)
#define FINGERPRINT_RESPONSE_MSG_LEN            (23)
#define FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT    (0xFF)
#define FINGERPRINT_SING_ERR_TIMES              (3)
#define FINGERPRINT_SING_DISABLE_ERR_TIMES      (15)
#endif

#define QRCODE_CONFIRM_SIGN_PROCESS 66
#define FINGER_SIGN_MAX_COUNT 5

typedef struct QrCodeWidget {
    lv_obj_t *cont;
    lv_obj_t *analysis;
} QrCodeWidget_t;

typedef enum {
    PAGE_PHASE_SCAN_QR,
    PAGE_PHASE_TRANSACTION_DETAIL,
    PAGE_PHASE_SIGNATURE
} PagePhase;

void OpenForgetPasswordHandler(lv_event_t *e);

static QrCodeWidget_t g_qrCodeWidgetView;
static ViewType g_qrcodeViewType;
static lv_obj_t *g_fingerSingContainer = NULL;
static lv_obj_t *g_fpErrorImg = NULL;
static lv_obj_t *g_fpErrorLabel = NULL;
static uint32_t g_fingerSignCount = FINGER_SIGN_MAX_COUNT;
static uint32_t g_fingerSignErrCount = 0;
static lv_obj_t *g_scanErrorHintBox = NULL;

static uint8_t g_chainType = CHAIN_BUTT;

static lv_timer_t *g_fpRecognizeTimer;

static KeyboardWidget_t *g_keyboardWidget = NULL;
static PagePhase g_pagePhase;
static PageWidget_t *g_pageWidget;

void GuiQrCodeScreenCorner(void)
{
    UpdatePageContentZone(g_pageWidget);
    g_qrCodeWidgetView.cont = g_pageWidget->contentZone;
    lv_obj_t *cont = g_qrCodeWidgetView.cont;
    printf("qrcode refresh...\n");
    static lv_point_t topLinePoints[2] = {{0, 0}, {322, 0}};
    static lv_point_t bottomLinePoints[2] = {{0, 0}, {322, 0}};
    static lv_point_t leftLinePoints[2] = {{0, 0}, {0, 322}};
    static lv_point_t rightLinePoints[2] = {{0, 0}, {0, 322}};
    lv_obj_t *line = GuiCreateLine(cont, topLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 80, 224 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateLine(cont, bottomLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_BOTTOM_LEFT, 80, -254);
    line = GuiCreateLine(cont, leftLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 80, 224 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateLine(cont, rightLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_BOTTOM_RIGHT, -78, -254);

    lv_obj_t *img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 80, 223 - GUI_MAIN_AREA_OFFSET);

    img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -77 + 28, 223 - GUI_MAIN_AREA_OFFSET - 1);
    lv_img_set_angle(img, 900);
    lv_img_set_pivot(img, 0, 0);

    img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 79, -254 + 28 + 1);
    lv_img_set_angle(img, -900);
    lv_img_set_pivot(img, 0, 0);

    img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_BOTTOM_RIGHT, -77 + 28, -254 + 28 + 1);
    lv_img_set_angle(img, 1800);
    lv_img_set_pivot(img, 0, 0);
}

void GuiQrCodeScreenInit(void *param)
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_pageWidget = CreatePageWidget();
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    g_pagePhase = PAGE_PHASE_SCAN_QR;
    SetPageLockScreen(false);
}


static void SignByPasswordCb(bool cancel)
{
    GUI_DEL_OBJ(g_fingerSingContainer)
    if (cancel) {
        FpCancelCurOperate();
    }

    g_keyboardWidget = GuiCreateKeyboardWidget(g_qrCodeWidgetView.cont);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
}

static void SignByPasswordCbHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        SignByPasswordCb(true);
    }
}

static void CloseContHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_fingerSingContainer)
    }
}

void SignByFinger(void)
{
    GUI_DEL_OBJ(g_fingerSingContainer)

    g_fingerSingContainer = GuiCreateHintBox(lv_scr_act(), 480, 428, true);
    lv_obj_t *cont = g_fingerSingContainer;
    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("scan_qr_code_sign_fingerprint_verify_fingerprint"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 402);

    lv_obj_t *img = GuiCreateImg(cont, &imgClose);
    GuiButton_t table[2] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {14, 14},}
    };
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

void CheckSliderProcessHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED) {
        int32_t value = lv_slider_get_value(lv_event_get_target(e));
        if (value >= QRCODE_CONFIRM_SIGN_PROCESS) {
            if ((GetCurrentAccountIndex() < 3) && GetFingerSignFlag() && g_fingerSignCount < 3) {
                SignByFinger();
            } else {
                SignByPasswordCb(false);
            }
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_OFF);
        } else {
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_ON);
        }
    }
}

static GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t ViewType)
{
    switch (ViewType) {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
        return CHAIN_BTC;
    case LtcTx:
        return CHAIN_LTC;
    case DashTx:
        return CHAIN_DASH;
    case BchTx:
        return CHAIN_BCH;
    case EthPersonalMessage:
    case EthTx:
    case EthTypedData:
        return CHAIN_ETH;
    case TronTx:
        return CHAIN_TRX;
    case CosmosTx:
    case CosmosEvmTx:
        return GuiGetCosmosTxChain();
    case SuiTx:
        return CHAIN_SUI;
    case SolanaTx:
    case SolanaMessage:
        return CHAIN_SOL;
    default:
        return CHAIN_BUTT;
    }
    return CHAIN_BUTT;
}

void CloseScanErrorDataHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_scanErrorHintBox)
        GuiQrCodeRefresh();
    }
}

static void GuiDealScanErrorResult(int errorType)
{
    g_scanErrorHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_scanErrorHintBox, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_scanErrorHintBox, _("scan_qr_code_error_invalid_qrcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_scanErrorHintBox, _("scan_qr_code_error_invalid_qrcode_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_scanErrorHintBox, _("OK"), &openSansEnText);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, CloseScanErrorDataHandler, LV_EVENT_CLICKED, NULL);
}

void GuiQrCodeScanResult(bool result, void *param)
{
    if (result) {
        UrViewType_t urViewType = *(UrViewType_t *)param;
        g_qrcodeViewType = urViewType.viewType;
        g_chainType = ViewTypeToChainTypeSwitch(g_qrcodeViewType);
        // Not a chain based transaction, e.g. WebAuth
        if (g_chainType == CHAIN_BUTT) {
            if (g_qrcodeViewType == WebAuthResult) {
                GuiCLoseCurrentWorkingView();
                GuiFrameOpenView(&g_webAuthResultView);
            }
            return;
        }
        uint8_t accountNum = 0;
        GetExistAccountNum(&accountNum);
        if (accountNum <= 0) {
            GuiQrCodeScreenCorner();
            GuiDealScanErrorResult(0);
            return;
        }
        g_qrCodeWidgetView.analysis = GuiTemplateReload(g_qrCodeWidgetView.cont, g_qrcodeViewType);
        if (g_qrCodeWidgetView.analysis != NULL) {
            g_fingerSignCount = 0;
            if (g_qrcodeViewType == EthPersonalMessage || g_qrcodeViewType == EthTypedData || IsCosmosMsg(g_qrcodeViewType)) {
                SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("transaction_parse_confirm_message"));
            } else {
                SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, NULL);
            }
            GuiCreateConfirmSlider(g_qrCodeWidgetView.cont, CheckSliderProcessHandler);
            g_pagePhase = PAGE_PHASE_TRANSACTION_DETAIL;
            SetPageLockScreen(true);
        } else {
            GuiDealScanErrorResult(0);
        }
    } else {
        GuiQrCodeScreenCorner();
        GuiDealScanErrorResult(0);
    }
}

lv_obj_t* GuiCreateQRCode(lv_obj_t* parent, uint16_t w, uint16_t h)
{
    lv_obj_t* qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    return qrcode;
}

lv_res_t UpdateQrCodeAndFullscreenVersion(lv_obj_t * qrcode, const void * data, uint32_t data_len)
{
    lv_qrcode_update(qrcode, data, data_len);
    lv_obj_t* fullScreenQrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullScreenQrcode) {
        lv_qrcode_update(fullScreenQrcode, data, data_len);
    }
}

void GuiShowQrCode(GetUR func, lv_obj_t *qr)
{
    UpdateQrCode(func, qr, UpdateQrCodeAndFullscreenVersion);
}

void GuiQrCodeShowQrMessage(lv_obj_t *parent)
{
    g_pagePhase = PAGE_PHASE_SIGNATURE;
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(cont, LV_ALIGN_DEFAULT);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(cont, 408, 408);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);
    lv_obj_t *qrcode = GuiCreateQRCode(qrCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(GuiCreateQRCode, 420, 420);

    char *data = NULL;
    switch (g_qrcodeViewType) {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
    case LtcTx:
    case DashTx:
    case BchTx:
        GuiShowQrCode(GuiGetSignQrCodeData, qrcode);
        break;
    case EthTx:
    case EthPersonalMessage:
    case EthTypedData:
        GuiShowQrCode(GuiGetEthSignQrCodeData, qrcode);
        break;
    case TronTx:
        GuiShowQrCode(GuiGetTrxSignQrCodeData, qrcode);
        break;
    case CosmosTx:
    case CosmosEvmTx:
        GuiShowQrCode(GuiGetCosmosSignQrCodeData, qrcode);
        break;
    case SuiTx:
        GuiShowQrCode(GuiGetSuiSignQrCodeData, qrcode);
        break;
    case SolanaTx:
    case SolanaMessage:
        GuiShowQrCode(GuiGetSolSignQrCodeData, qrcode);
        break;
    default:
        data = "";
        lv_qrcode_update(qrcode, data, strlen(data));
        lv_obj_t* fullScreenQrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
        if (fullScreenQrcode) {
            lv_qrcode_update(fullScreenQrcode, data, strlen(data));
        }
        break;
    }

    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("transaction_parse_scan_by_software"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 576 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *btn = GuiCreateBtn(cont, _("Done"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_event_cb(btn, CloseTimerCurrentViewHandler, LV_EVENT_CLICKED, NULL);

    uint8_t chainType = ViewTypeToChainTypeSwitch(g_qrcodeViewType);
    if (g_qrcodeViewType == EthPersonalMessage || g_qrcodeViewType == EthTypedData) {
        SetCoinWallet(g_pageWidget->navBarWidget, chainType, _("transaction_parse_broadcast_message"));
    }
}

void GuiQrCodeVerifyPasswordSuccess(void)
{
    lv_obj_del(g_qrCodeWidgetView.analysis);
    GUI_DEL_OBJ(g_fingerSingContainer)
    GUI_DEL_OBJ(g_scanErrorHintBox)
    g_qrCodeWidgetView.analysis = NULL;
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GuiQrCodeShowQrMessage(g_qrCodeWidgetView.cont);
}

void GuiQrCodeRefresh(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseTimerCurrentViewHandler, NULL);
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    switch (g_pagePhase) {
    case PAGE_PHASE_SCAN_QR:
        GuiQrCodeScreenCorner();
        GuiModeControlQrDecode(true);
        break;
    case PAGE_PHASE_TRANSACTION_DETAIL:
        if (g_qrcodeViewType == EthPersonalMessage || g_qrcodeViewType == EthTypedData || IsCosmosMsg(g_qrcodeViewType)) {
            SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("transaction_parse_confirm_message"));
        } else {
            SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, NULL);
        }
        break;
    case PAGE_PHASE_SIGNATURE:
        if (g_qrcodeViewType == EthPersonalMessage || g_qrcodeViewType == EthTypedData) {
            SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("transaction_parse_broadcast_message"));
        } else {
            SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, NULL);
        }
        break;
    default:
        break;
    }
}

void GuiQrCodeDeInit(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GUI_DEL_OBJ(g_scanErrorHintBox)
    GUI_DEL_OBJ(g_fingerSingContainer)
    GuiTemplateClosePage();
    CloseQRTimer();
    lv_obj_del(g_qrCodeWidgetView.cont);
    g_qrCodeWidgetView.cont = NULL;
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_chainType = CHAIN_BUTT;
    g_pagePhase = PAGE_PHASE_SCAN_QR;

    //for learn more hintbox in eth contract data block;
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }

    GuiFullscreenModeCleanUp();
    SetPageLockScreen(true);
}

static void RecognizeFailHandler(lv_timer_t *timer)
{
    if (g_fingerSingContainer != NULL) {
        lv_img_set_src(g_fpErrorImg, &imgYellowFinger);
        lv_obj_add_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);
    }
    lv_timer_del(timer);
    g_fpRecognizeTimer = NULL;
}

void GuiClearQrcodeSignCnt(void)
{
    g_fingerSignErrCount = 0;
}

void GuiQrCodeDealFingerRecognize(void *param)
{
    uint8_t errCode = *(uint8_t *)param;
    static uint16_t passCodeType = ENTER_PASSCODE_VERIFY_PASSWORD;
    if (g_fingerSingContainer == NULL) {
        return;
    }
    if (errCode == FP_SUCCESS_CODE) {
        lv_img_set_src(g_fpErrorImg, &imgYellowFinger);
        GuiModelVerifyAmountPassWord(&passCodeType);
        g_fingerSignErrCount = 0;
    } else {
        g_fingerSignErrCount++;
        g_fingerSignCount++;
        if (g_fpErrorLabel != NULL && lv_obj_has_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);
        }
        lv_img_set_src(g_fpErrorImg, &imgRedFinger);
        printf("GuiQrCodeDealFingerRecognize err message is %s\n", GetFpErrorMessage(errCode));
        printf("g_fingerSingCount is %d\n", g_fingerSignCount);
        if (g_fingerSignCount < FINGERPRINT_SING_ERR_TIMES) {
            FpRecognize(RECOGNIZE_SIGN);
            g_fpRecognizeTimer = lv_timer_create(RecognizeFailHandler, 1000, NULL);
        } else {
            SignByPasswordCb(false);
        }
        printf("g_fingerSignErrCount.... = %d\n", g_fingerSignErrCount);
        if (g_fingerSignErrCount >= FINGERPRINT_SING_DISABLE_ERR_TIMES) {
            for (int i = 0; i < 3;  i++) {
                UpdateFingerSignFlag(i, false);
            }
        }
    }
}

void GuiQrCodeVerifyPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}
