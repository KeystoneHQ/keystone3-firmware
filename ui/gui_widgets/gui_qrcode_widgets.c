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
#ifndef COMPILE_SIMULATOR
#include "fingerprint_process.h"
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
    lv_obj_t *errLabel;
    KeyBoard_t *kb;
} QrCodeWidget_t;

void OpenForgetPasswordHandler(lv_event_t *e);

static QrCodeWidget_t g_qrCodeWidgetView;
static ViewType g_qrcodeViewType;
static lv_obj_t *g_fingerSingContainer = NULL;
static lv_obj_t *g_fpErrorImg = NULL;
static lv_obj_t *g_fpErrorLabel = NULL;
static uint32_t g_fingerSignCount = FINGER_SIGN_MAX_COUNT;
static uint32_t g_fingerSignErrCount = 0;
static lv_obj_t *g_noticeHintBox = NULL;

static lv_obj_t *g_errorHintBox = NULL;
static lv_timer_t *g_countDownTimer;
static int8_t countDown = 5;
static uint8_t g_chainType = CHAIN_BUTT;

static void UnlockDeviceHandler(lv_event_t *e);
static void GuiShowPasswordErrorHintBox(void);
static void CountDownTimerWipeDeviceHandler(lv_timer_t *timer);
static void GuiHintBoxToLockSreen(void);
static void GuiCountDownDestruct(void *obj, void *param);
static lv_timer_t *g_fpRecognizeTimer;

void GuiQrCodeScreenCorner(void)
{
    if (g_qrCodeWidgetView.cont != NULL) {
        lv_obj_del(g_qrCodeWidgetView.cont);
        g_qrCodeWidgetView.cont = NULL;
    }
    g_qrCodeWidgetView.cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_align(g_qrCodeWidgetView.cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
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
    g_qrCodeWidgetView.kb = NULL;
}

static void UpdatePassPhraseHandler(lv_event_t *e)
{
    // static bool delayFlag = false;
    static uint16_t passCodeType = ENTER_PASSCODE_VERIFY_PASSWORD;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        const char *currText = lv_textarea_get_text(g_qrCodeWidgetView.kb->ta);
        if (strlen(currText) > 0) {
            SecretCacheSetPassword((char *)currText);
            GuiModelVerifyAmountPassWord(&passCodeType);
            lv_textarea_set_text(g_qrCodeWidgetView.kb->ta, "");
        }
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (!lv_obj_has_flag(g_qrCodeWidgetView.errLabel, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(g_qrCodeWidgetView.errLabel, LV_OBJ_FLAG_HIDDEN);
        }
        Vibrate(SLIGHT);
    }
}

static void ForgetHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeHintBox)
        OpenForgetPasswordHandler(e);
    }
}

static void SignByPasswordCb(bool cancel)
{
    GUI_DEL_OBJ(g_fingerSingContainer)
    if (cancel) {
        FpCancelCurOperate();
    }

    g_noticeHintBox = GuiCreateHintBox(g_qrCodeWidgetView.cont, 480, 576, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    lv_obj_t *label = GuiCreateIllustrateLabel(g_noticeHintBox, _("Please Enter Passcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 254);

    lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgClose);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, NULL);
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_CENTER, .position = {0, 0},},
    };
    lv_obj_t *button = GuiCreateButton(g_noticeHintBox, 36, 36, table, NUMBER_OF_ARRAYS(table),
                                       CloseHintBoxHandler, &g_noticeHintBox);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 408, 251);

    g_qrCodeWidgetView.kb = GuiCreateFullKeyBoard(g_noticeHintBox, UpdatePassPhraseHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_qrCodeWidgetView.kb, 0);
    lv_obj_t *ta = g_qrCodeWidgetView.kb->ta;
    lv_textarea_set_placeholder_text(ta, _("Enter Passcode"));
    lv_obj_set_size(ta, 352, 100);
    lv_obj_align(ta, LV_ALIGN_DEFAULT, 36, 332);
    lv_obj_set_style_text_opa(ta, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ta, DARK_BG_COLOR, LV_PART_MAIN);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_max_length(ta, GUI_DEFINE_MAX_PASSCODE_LEN);
    lv_textarea_set_one_line(ta, true);

    img = GuiCreateImg(g_noticeHintBox, &imgEyeOff);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 411, 332);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, ta);

    button = GuiCreateImgLabelButton(g_noticeHintBox, _("FORGET"), &imgLock, ForgetHandler, &g_qrCodeView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 333, 439);

    label = GuiCreateIllustrateLabel(g_noticeHintBox, _("Password does not match"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 390);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_qrCodeWidgetView.errLabel = label;
    lv_label_set_recolor(g_qrCodeWidgetView.errLabel, true);
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
    lv_obj_t *label = GuiCreateNoticeLabel(cont, "Verify Fingerprint");
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

    g_fpErrorLabel = GuiCreateLabel(cont, "Verify failed. Please try again!");
    lv_obj_set_style_text_color(g_fpErrorLabel, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(g_fpErrorLabel, LV_ALIGN_BOTTOM_MID, 0, -100);
    lv_obj_add_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);

    label = GuiCreateNoticeLabel(cont, "Enter Password");
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
    default:
        return CHAIN_BUTT;
    }
    return CHAIN_BUTT;
}

void CloseScanErrorDataHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeHintBox)
        GuiQrCodeRefresh();
    }
}

static void GuiDealScanErrorResult(int errorType)
{
    g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("Invalid QR Code"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeHintBox, _("QR code data not recognized. Please try again."));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeHintBox, _("OK"), &openSansEnText);
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
        g_qrCodeWidgetView.analysis = GuiTemplateReload(g_qrCodeWidgetView.cont, g_qrcodeViewType);
        if (g_qrCodeWidgetView.analysis != NULL) {
            g_fingerSignCount = 0;
            if (g_qrcodeViewType == EthPersonalMessage || g_qrcodeViewType == EthTypedData) {
                GuiNvsSetCoinWallet(g_chainType, "Confirm Message");
            } else {
                GuiNvsSetCoinWallet(g_chainType, NULL);
            }
            GuiCreateConfirmSlider(g_qrCodeWidgetView.cont, CheckSliderProcessHandler);
        } else {
            GuiDealScanErrorResult(0);
        }
    } else {
        GuiQrCodeScreenCorner();
        GuiDealScanErrorResult(0);
    }
}

void GuiQrCodeShowQrMessage(lv_obj_t *parent)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(cont, LV_ALIGN_DEFAULT);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(cont, 408, 408);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);
    lv_obj_t *qrcode = lv_qrcode_create(qrCont, 336, BLACK_COLOR, WHITE_COLOR);

    char *data = NULL;
    switch (g_qrcodeViewType) {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
    case LtcTx:
    case DashTx:
    case BchTx:
        ShowQrCode(GuiGetSignQrCodeData, qrcode);
        break;
    case EthTx:
    case EthPersonalMessage:
    case EthTypedData:
        ShowQrCode(GuiGetEthSignQrCodeData, qrcode);
        break;
    case TronTx:
        ShowQrCode(GuiGetTrxSignQrCodeData, qrcode);
        break;
    default:
        data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
        lv_qrcode_update(qrcode, data, strlen(data));
        break;
    }

    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("Scan the QR code with your software wallet"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 576 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *btn = GuiCreateBtn(cont, _("Done"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_event_cb(btn, CloseTimerCurrentViewHandler, LV_EVENT_CLICKED, NULL);

    uint8_t chainType = ViewTypeToChainTypeSwitch(g_qrcodeViewType);
    if (g_qrcodeViewType == EthPersonalMessage || g_qrcodeViewType == EthTypedData) {
        GuiNvsSetCoinWallet(chainType, "Broadcast Message");
    }
}

void GuiQrCodeVerifyPasswordResult(bool result)
{
    if (result) {
        lv_obj_del(g_qrCodeWidgetView.analysis);
        GUI_DEL_OBJ(g_fingerSingContainer)
        g_qrCodeWidgetView.analysis = NULL;
        GUI_DEL_OBJ(g_noticeHintBox)
        GuiQrCodeShowQrMessage(g_qrCodeWidgetView.cont);
    } else {
        lv_obj_clear_flag(g_qrCodeWidgetView.errLabel, LV_OBJ_FLAG_HIDDEN);
        if (g_qrCodeWidgetView.kb != NULL) {
            lv_textarea_set_text(g_qrCodeWidgetView.kb->ta, "");
        }
    }
}

void GuiQrCodeRefresh(void)
{
    g_noticeHintBox = NULL;
    GuiNvsBarSetLeftCb(NVS_BAR_RETURN, CloseTimerCurrentViewHandler, NULL);
    GuiNvsBarSetMidCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    if (g_qrCodeWidgetView.kb == NULL) {
        g_noticeHintBox = NULL;
        GuiQrCodeScreenCorner();
        GuiModeControlQrDecode(true);
    } else {
        if (g_qrcodeViewType == EthPersonalMessage || g_qrcodeViewType == EthTypedData) {
            GuiNvsSetCoinWallet(g_chainType, "Confirm Message");
        } else {
            GuiNvsSetCoinWallet(g_chainType, NULL);
        }
    }
}

void GuiQrCodeDeInit(void)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    GUI_DEL_OBJ(g_fingerSingContainer)
    GuiTemplateClosePage();
    CloseQRTimer();
    lv_obj_del(g_qrCodeWidgetView.cont);
    g_qrCodeWidgetView.cont = NULL;
    g_qrCodeWidgetView.kb = NULL;
    g_chainType = CHAIN_BUTT;

    //for learn more hintbox in eth contract data block;
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
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
    printf("GuiQrCodeVerifyPasswordErrorCount  errorcount is %d\n", passwordVerifyResult->errorCount);

    if (g_qrCodeWidgetView.errLabel != NULL) {
        char hint[128];
        sprintf(hint, "Incorrect password, you have #F55831 %d# chances left", (MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX - passwordVerifyResult->errorCount));
        lv_label_set_text(g_qrCodeWidgetView.errLabel, hint);
        if (passwordVerifyResult->errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
            GuiShowPasswordErrorHintBox();
        }
    }
}

static void GuiShowPasswordErrorHintBox(void)
{
    if (g_errorHintBox == NULL) {
        g_errorHintBox = GuiCreateResultHintbox(lv_scr_act(), 386, &imgFailed,
                                                "Attempt Limit Exceeded", "Device lock imminent. Please unlock to access the device.",
                                                NULL, DARK_GRAY_COLOR, "Unlock Device (5s)", DARK_GRAY_COLOR);
    }

    if (g_errorHintBox != NULL) {
        lv_obj_set_parent(g_errorHintBox, lv_scr_act());
        if (lv_obj_has_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_t *btn = GuiGetHintBoxRightBtn(g_errorHintBox);
        lv_label_set_text(lv_obj_get_child(btn, 0), "Unlock Device (5s)");

        lv_obj_remove_event_cb(btn, UnlockDeviceHandler);
        lv_obj_add_event_cb(btn, UnlockDeviceHandler, LV_EVENT_CLICKED, NULL);
        g_countDownTimer = lv_timer_create(CountDownTimerWipeDeviceHandler, 1000, btn);
    }
}

static void UnlockDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiHintBoxToLockSreen();
        GuiCountDownDestruct(NULL, NULL);
    }
}

static void GuiHintBoxToLockSreen(void)
{
    static uint16_t sig = SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS;
    GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_UNLOCK);
    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &sig, sizeof(sig));

    if (!lv_obj_has_flag(g_qrCodeWidgetView.errLabel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(g_qrCodeWidgetView.errLabel, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_errorHintBox != NULL) {
        if (!lv_obj_has_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void GuiCountDownDestruct(void *obj, void *param)
{
    if (g_countDownTimer != NULL) {
        countDown = 5;
        lv_timer_del(g_countDownTimer);
        g_countDownTimer = NULL;
        UNUSED(g_countDownTimer);
    }
}

static void CountDownTimerWipeDeviceHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    char buf[32] = {0};
    --countDown;
    if (countDown > 0) {
        sprintf(buf, "Unlock Device (%ds)", countDown);
    } else {
        strcpy(buf, "Unlock Device");
    }
    lv_label_set_text(lv_obj_get_child(obj, 0), buf);
    if (countDown <= 0) {
        GuiHintBoxToLockSreen();
        GuiCountDownDestruct(NULL, NULL);
    }
}