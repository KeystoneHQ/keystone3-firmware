#include <stdio.h>
#include "gui.h"
#include "gui_obj.h"
#include "user_memory.h"
#include "gui_views.h"
#include "gui_hintbox.h"
#include "gui_button.h"
#include "gui_enter_passcode.h"
#include "gui_keyboard_hintbox.h"
#include "assert.h"
#include "motor_manager.h"
#include "secret_cache.h"
#include "gui_views.h"
#include "gui_lock_widgets.h"
#include "fingerprint_process.h"
#ifdef COMPILE_SIMULATOR
#define RECOGNIZE_UNLOCK                    0
#endif
#define DEFAULT_TIMER_COUNTER 5

static KeyboardWidget_t *CreateKeyboardWidget();

static void KeyboardConfirmHandler(lv_event_t *e);
static void ForgetHandler(lv_event_t *e);
static void CloseKeyBoardWidgetHandler(lv_event_t *e);

static void GuiShowPasswordErrorHintBox(KeyboardWidget_t *keyboardWidget);
static void LockDeviceHandler(lv_event_t *e);
static void GuiHintBoxToLockSreen(void);
static void CountDownHandler(lv_timer_t *timer);
static void GuiCountDownTimerDestruct(KeyboardWidget_t *keyboardWidget);


static void KeyboardConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_READY) {
        const char *currText = GuiGetKeyboardInput(keyboardWidget);
        if (strlen(currText) > 0) {
            SecretCacheSetPassword((char *)currText);
            GuiModelVerifyAmountPassWord(keyboardWidget->sig);
            GuiClearKeyboardInput(keyboardWidget);
        }
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        GuiHideErrorLabel(keyboardWidget);
        Vibrate(SLIGHT);
    }
}

static void ForgetHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {

        GuiFrameOpenView(&g_forgetPassView);
    }
}


static void CloseKeyBoardWidgetHandler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)lv_event_get_user_data(e);
        GuiDeleteKeyboardWidget(keyboardWidget);
    }
}

static KeyboardWidget_t *CreateKeyboardWidget()
{

    KeyboardWidget_t *keyboardWidget = SRAM_MALLOC(sizeof(KeyboardWidget_t));
    keyboardWidget->keyboardHintBox = NULL;
    keyboardWidget->kb = NULL;
    keyboardWidget->errLabel = NULL;
    keyboardWidget->sig = ENTER_PASSCODE_VERIFY_PASSWORD;
    keyboardWidget->countDownTimer = NULL;
    keyboardWidget->timerCounter = SRAM_MALLOC(sizeof(uint8_t));
    *keyboardWidget->timerCounter = DEFAULT_TIMER_COUNTER;
    keyboardWidget->errHintBox = NULL;
    keyboardWidget->errHintBoxBtn = NULL;
    keyboardWidget->self = NULL;
    return keyboardWidget;
}

void SetKeyboardWidgetSig(KeyboardWidget_t *keyboardWidget, uint16_t *sig)
{
    keyboardWidget->sig = sig;
}

void SetKeyboardWidgetSelf(KeyboardWidget_t *keyboardWidget, KeyboardWidget_t **self)
{
    keyboardWidget->self = self;
}


KeyboardWidget_t *GuiCreateKeyboardWidget(lv_obj_t *parent)
{
    KeyboardWidget_t *keyboardWidget = CreateKeyboardWidget();

    lv_obj_t *keyboardHintBox = GuiCreateHintBox(parent, 480, 576, true);

    lv_obj_add_event_cb(lv_obj_get_child(keyboardHintBox, 0), CloseKeyBoardWidgetHandler, LV_EVENT_CLICKED, keyboardWidget);

    lv_obj_t *label = GuiCreateIllustrateLabel(keyboardHintBox, _("enter_passcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 254);

    lv_obj_t *img = GuiCreateImg(keyboardHintBox, &imgClose);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, NULL);
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_CENTER, .position = {0, 0},},
    };
    lv_obj_t *button = GuiCreateButton(keyboardHintBox, 36, 36, table, NUMBER_OF_ARRAYS(table),
                                       CloseKeyBoardWidgetHandler, keyboardWidget);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 408, 251);

    keyboardWidget->keyboardHintBox = keyboardHintBox;

    KeyBoard_t *kb = GuiCreateFullKeyBoard(keyboardHintBox, KeyboardConfirmHandler, KEY_STONE_FULL_L, keyboardWidget);
    lv_obj_t *ta = kb->ta;
    lv_textarea_set_placeholder_text(ta, _("Enter Passcode"));
    lv_obj_set_size(ta, 352, 100);
    lv_obj_align(ta, LV_ALIGN_DEFAULT, 36, 332);
    lv_obj_set_style_text_opa(ta, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ta, DARK_BG_COLOR, LV_PART_MAIN);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_max_length(ta, GUI_DEFINE_MAX_PASSCODE_LEN);
    lv_textarea_set_one_line(ta, true);

    keyboardWidget->kb = kb;

    img = GuiCreateImg(keyboardHintBox, &imgEyeOff);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 411, 332);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, ta);

    button = GuiCreateImgLabelButton(keyboardHintBox, _("FORGET"), &imgLock, ForgetHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 333, 439);

    label = GuiCreateIllustrateLabel(keyboardHintBox, _("password_error_not_match"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 390);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_recolor(label, true);

    keyboardWidget->errLabel = label;

    return keyboardWidget;
}

void GuiDeleteKeyboardWidget(KeyboardWidget_t *keyboardWidget)
{
    if (keyboardWidget != NULL && keyboardWidget->self != NULL) {
        if (keyboardWidget->keyboardHintBox != NULL && lv_obj_is_valid(keyboardWidget->keyboardHintBox)) {
            lv_obj_del(keyboardWidget->keyboardHintBox);
            keyboardWidget->keyboardHintBox = NULL;
        }
        if (keyboardWidget->errHintBox != NULL && lv_obj_is_valid(keyboardWidget->errHintBox)) {
            lv_obj_del(keyboardWidget->errHintBox);
            keyboardWidget->errHintBox = NULL;
        }
        GuiCountDownTimerDestruct(keyboardWidget);

        if (keyboardWidget->timerCounter != NULL) {
            SRAM_FREE(keyboardWidget->timerCounter);
            keyboardWidget->timerCounter = NULL;
        }

        *(keyboardWidget->self) = NULL;
        SRAM_FREE(keyboardWidget);
    }
}

const char *GuiGetKeyboardInput(KeyboardWidget_t *keyboardWidget)
{
    assert(keyboardWidget);
    assert(keyboardWidget->kb);
    assert(keyboardWidget->kb->ta);
    return lv_textarea_get_text(keyboardWidget->kb->ta);
}

void GuiClearKeyboardInput(KeyboardWidget_t *keyboardWidget)
{
    assert(keyboardWidget);
    assert(keyboardWidget->kb);
    assert(keyboardWidget->kb->ta);
    lv_textarea_set_text(keyboardWidget->kb->ta, "");
}

void GuiSetErrorLabel(KeyboardWidget_t *keyboardWidget, char *errorMessage)
{
    if (keyboardWidget != NULL && keyboardWidget->errLabel != NULL) {
        lv_label_set_text(keyboardWidget->errLabel, errorMessage);
        lv_obj_clear_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiShowErrorLabel(KeyboardWidget_t *keyboardWidget)
{
    if (lv_obj_has_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiHideErrorLabel(KeyboardWidget_t *keyboardWidget)
{
    if (!lv_obj_has_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
    }
}


void GuiShowErrorNumber(KeyboardWidget_t *keyboardWidget, PasswordVerifyResult_t *passwordVerifyResult)
{
    printf("GuiShowErrorNumber error count is %d\n", passwordVerifyResult->errorCount);
    char hint[128];
    sprintf(hint, _("unlock_device_attempts_left_times_fmt"), (MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX - passwordVerifyResult->errorCount));
    GuiSetErrorLabel(keyboardWidget, hint);
    if (passwordVerifyResult->errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
        GuiShowPasswordErrorHintBox(keyboardWidget);
    }
}

static void GuiShowPasswordErrorHintBox(KeyboardWidget_t *keyboardWidget)
{

    lv_obj_t *errHintBox = GuiCreateResultHintbox(lv_scr_act(), 386, &imgFailed,
                           _("unlock_device_error_attempts_exceed"), _("unlock_device_error_attempts_exceed_desc"),
                           NULL, DARK_GRAY_COLOR, _("unlock_device_error_btn_start_text"), DARK_GRAY_COLOR);

    lv_obj_t *btn = GuiGetHintBoxRightBtn(errHintBox);
    lv_label_set_text(lv_obj_get_child(btn, 0), _("unlock_device_error_btn_start_text"));
    lv_obj_add_event_cb(btn, LockDeviceHandler, LV_EVENT_CLICKED, keyboardWidget);
    lv_timer_t *countDownTimer = lv_timer_create(CountDownHandler, 1000, keyboardWidget);


    keyboardWidget->countDownTimer = countDownTimer;
    keyboardWidget->errHintBox = errHintBox;
    keyboardWidget->errHintBoxBtn = btn;
}

static void LockDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)lv_event_get_user_data(e);
        GuiHintBoxToLockSreen();
        GuiDeleteKeyboardWidget(keyboardWidget);
    }
}

static void GuiHintBoxToLockSreen(void)
{
    static uint16_t sig = SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS;
    GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_UNLOCK);
    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &sig, sizeof(sig));
    if (GuiNeedFpRecognize()) {
        FpRecognize(RECOGNIZE_UNLOCK);
    }
}

static void CountDownHandler(lv_timer_t *timer)
{
    KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)timer->user_data;

    char buf[32] = {0};
    --(*keyboardWidget->timerCounter);
    if (*keyboardWidget->timerCounter > 0) {
        sprintf(buf, _("unlock_device_error_btn_text_fmt"), *keyboardWidget->timerCounter);
    } else {
        strcpy(buf, _("unlock_device_error_btn_end_text"));
    }
    lv_label_set_text(lv_obj_get_child(keyboardWidget->errHintBoxBtn, 0), buf);

    if (*keyboardWidget->timerCounter <= 0) {
        GuiHintBoxToLockSreen();
        GuiDeleteKeyboardWidget(keyboardWidget);
    }
}

static void GuiCountDownTimerDestruct(KeyboardWidget_t *keyboardWidget)
{
    if (keyboardWidget != NULL && keyboardWidget->countDownTimer != NULL) {
        lv_timer_del(keyboardWidget->countDownTimer);
        keyboardWidget->countDownTimer = NULL;
    }
}