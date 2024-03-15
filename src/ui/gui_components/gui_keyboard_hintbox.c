#include <stdio.h>
#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_led.h"
#include "gui_hintbox.h"
#include "gui_button.h"
#include "gui_enter_passcode.h"
#include "gui_keyboard.h"
#include "gui_keyboard_hintbox.h"
#include "assert.h"
#include "user_memory.h"
#include "motor_manager.h"
#include "secret_cache.h"
#include "gui_views.h"
#include "gui_lock_widgets.h"
#include "fingerprint_process.h"
#include "gui_model.h"
#include "usb_task.h"
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#include "usb_task.h"
#else
#include "simulator_mock_define.h"
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

static uint8_t g_keyboardHintBoxMode = KEYBOARD_HINTBOX_PIN;
static char g_pinBuf[PASSWORD_MAX_LEN + 1];

void SetKeyboardWidgetMode(uint8_t mode)
{
    g_keyboardHintBoxMode = mode;
}

uint8_t GetKeyboardWidgetMode(void)
{
    return g_keyboardHintBoxMode;
}

static void KeyboardConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_READY) {
        const char *currText = GuiGetKeyboardInput(keyboardWidget);
        if (strnlen_s(currText, PASSWORD_MAX_LEN) > 0) {
            SecretCacheSetPassword((char *)currText);
            GuiClearKeyboardInput(keyboardWidget);
            GuiModelVerifyAccountPassWord(keyboardWidget->sig);
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
    keyboardWidget->btnm = NULL;
    for (int i = 0; i < 6; i++) {
        keyboardWidget->led[i] = NULL;
    }
    keyboardWidget->kb = NULL;
    keyboardWidget->errLabel = NULL;
    static uint16_t sig = ENTER_PASSCODE_VERIFY_PASSWORD;
    keyboardWidget->sig = &sig;
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

static void ClearKeyboardWidgetCache(KeyboardWidget_t *keyboardWidget)
{
    memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
    keyboardWidget->currentNum = 0;
    for (int i = 0; i < CREATE_PIN_NUM; i++) {
        GuiSetLedStatus(keyboardWidget->led[i], PASSCODE_LED_OFF);
    }
    GuiClearKeyboardInput(keyboardWidget);
}

static void SetPinEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_RELEASED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(keyboardWidget->btnm);
        if (id == 9) {
            return;
        }
        Vibrate(SLIGHT);
        const char *txt = lv_btnmatrix_get_btn_text(keyboardWidget->btnm, id);
        if (strcmp(txt, USR_SYMBOL_DELETE) == 0) {
            if (keyboardWidget->currentNum > 0) {
                --keyboardWidget->currentNum;
                GuiSetLedStatus(keyboardWidget->led[keyboardWidget->currentNum], PASSCODE_LED_OFF);
                g_pinBuf[keyboardWidget->currentNum] = '\0';
            } else {
                for (int i = 0; i < CREATE_PIN_NUM; i++) {
                    GuiSetLedStatus(keyboardWidget->led[i], PASSCODE_LED_OFF);
                }
            }
            if (!lv_obj_has_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            if (keyboardWidget->currentNum < CREATE_PIN_NUM) {
                snprintf_s(g_pinBuf + keyboardWidget->currentNum, PASSWORD_MAX_LEN - keyboardWidget->currentNum, "%s", txt);
                keyboardWidget->currentNum++;
                for (int i = 0; i < keyboardWidget->currentNum; i++) {
                    GuiSetLedStatus(keyboardWidget->led[i], PASSCODE_LED_ON);
                }
                if (!lv_obj_has_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
                    for (int i = 1; i < CREATE_PIN_NUM; i++) {
                        GuiSetLedStatus(keyboardWidget->led[i], PASSCODE_LED_OFF);
                    }
                }
            }
            if (keyboardWidget->currentNum == CREATE_PIN_NUM) {
                SecretCacheSetPassword((char *)g_pinBuf);
                memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
                keyboardWidget->currentNum = 0;
                GuiClearKeyboardInput(keyboardWidget);
                GuiModelVerifyAccountPassWord(keyboardWidget->sig);
            }

        }
    }
}

static void PassWordPinHintSwitch(KeyboardWidget_t *keyboardWidget, uint8_t keyboardMode)
{
    if (keyboardMode == KEYBOARD_HINTBOX_PIN) {
        lv_obj_clear_flag(keyboardWidget->btnm, LV_OBJ_FLAG_HIDDEN);
        for (int i = 0; i < CREATE_PIN_NUM; i++) {
            lv_obj_clear_flag(keyboardWidget->led[i], LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_add_flag(keyboardWidget->kb->cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(keyboardWidget->kb->ta, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(keyboardWidget->eyeImg, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(keyboardWidget->switchLabel, _("password_label"));
    } else {
        lv_textarea_set_password_mode(keyboardWidget->kb->ta, true);
        lv_img_set_src(keyboardWidget->eyeImg, &imgEyeOff);
        lv_obj_clear_flag(keyboardWidget->kb->cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(keyboardWidget->kb->ta, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(keyboardWidget->eyeImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(keyboardWidget->btnm, LV_OBJ_FLAG_HIDDEN);
        for (int i = 0; i < CREATE_PIN_NUM; i++) {
            lv_obj_add_flag(keyboardWidget->led[i], LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text(keyboardWidget->switchLabel, _("pin_label"));
    }
    ClearKeyboardWidgetCache(keyboardWidget);
    g_keyboardHintBoxMode = keyboardMode;
}

void PassWordPinHintRefresh(KeyboardWidget_t *keyboardWidget)
{
    if (keyboardWidget != NULL) {
        PassWordPinHintSwitch(keyboardWidget, g_keyboardHintBoxMode);
    }
}

static void PassWordPinSwitchHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        uint8_t keyboardMode = lv_obj_has_flag(keyboardWidget->btnm, LV_OBJ_FLAG_HIDDEN) ? KEYBOARD_HINTBOX_PIN : KEYBOARD_HINTBOX_PASSWORD;
        PassWordPinHintSwitch(keyboardWidget, keyboardMode);
    }
}

static void CloseKeyboardWidgetViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        KeyboardWidget_t *keyboardWidget = (KeyboardWidget_t *)lv_event_get_user_data(e);
        GuiDeleteKeyboardWidget(keyboardWidget);
    }
}

KeyboardWidget_t *GuiCreateKeyboardWidgetView(lv_obj_t *parent, lv_event_cb_t buttonCb, uint16_t *signal)
{
    KeyboardWidget_t *keyboardWidget = CreateKeyboardWidget();
    lv_obj_t *keyboardHintBox = GuiCreateContainerWithParent(parent, 480, 800 - GUI_STATUS_BAR_HEIGHT);
    lv_obj_align(keyboardHintBox, LV_ALIGN_DEFAULT, 0, 0);

    lv_obj_t *img = GuiCreateImg(keyboardHintBox, &imgArrowLeft);
    if (*signal == SIG_FINGER_REGISTER_ADD_SUCCESS) {
        lv_img_set_src(img, &imgClose);
    }
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {14, 14},}
    };
    lv_obj_t *button = GuiCreateButton(keyboardHintBox, 64, 64, table, NUMBER_OF_ARRAYS(table), buttonCb ? buttonCb : CloseKeyboardWidgetViewHandler, keyboardWidget);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 10, 16);

    lv_obj_t *label = GuiCreateTitleLabel(keyboardHintBox, _("change_passcode_mid_btn"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12 + GUI_NAV_BAR_HEIGHT);
    label = GuiCreateNoticeLabel(keyboardHintBox, _("passphrase_add_password"));
    if (*signal == SIG_FINGER_REGISTER_ADD_SUCCESS) {
        lv_label_set_text(label, _("fingerprint_add_password"));
    }
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 72 + GUI_NAV_BAR_HEIGHT);

    keyboardWidget->keyboardHintBox = keyboardHintBox;

    KeyBoard_t *kb = GuiCreateFullKeyBoard(keyboardHintBox, KeyboardConfirmHandler, KEY_STONE_FULL_L, keyboardWidget);
    lv_obj_t *ta = kb->ta;
    lv_textarea_set_placeholder_text(ta, _("change_passcode_mid_btn"));
    lv_obj_set_size(ta, 352, 100);
    lv_obj_align(ta, LV_ALIGN_DEFAULT, 36, 260);
    lv_obj_set_style_text_opa(ta, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ta, BLACK_COLOR, LV_PART_MAIN);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_max_length(ta, PASSWORD_MAX_LEN);
    lv_textarea_set_one_line(ta, true);

    keyboardWidget->kb = kb;

    img = GuiCreateImg(keyboardHintBox, &imgEyeOff);
    lv_obj_align_to(img, ta, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, ta);
    keyboardWidget->eyeImg = img;

    if (*signal != SIG_FINGER_REGISTER_ADD_SUCCESS) {
        button = GuiCreateImgLabelButton(keyboardHintBox, _("FORGET"), &imgLock, 124, ForgetHandler, NULL);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 333, 439 - GUI_STATUS_BAR_HEIGHT);
    }

    button = GuiCreateImgLabelButton(keyboardHintBox, _("password_label"), &imgSwitch, 156, PassWordPinSwitchHandler, keyboardWidget);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 439 - GUI_STATUS_BAR_HEIGHT);
    keyboardWidget->switchLabel = lv_obj_get_child(button, 1);

    label = GuiCreateIllustrateLabel(keyboardHintBox, _("password_error_not_match"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 390 - GUI_STATUS_BAR_HEIGHT);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_recolor(label, true);

    keyboardWidget->errLabel = label;

    for (int i = 0; i < CREATE_PIN_NUM; i++) {
        lv_obj_t *led = GuiCreateLed(keyboardHintBox);
        lv_obj_align(led, LV_ALIGN_DEFAULT, 36 + 30 * i, 194 + GUI_NAV_BAR_HEIGHT);
        keyboardWidget->led[i] = led;
    }
    keyboardWidget->currentNum = 0;

    lv_obj_t *btnm = GuiCreateNumKeyboard(keyboardHintBox, SetPinEventHandler, NUM_KEYBOARD_PIN, keyboardWidget);
    lv_obj_add_style(btnm, &g_enterPressBtnmStyle, LV_STATE_PRESSED | LV_PART_ITEMS);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 0);
    keyboardWidget->btnm = btnm;

    PassWordPinHintSwitch(keyboardWidget, g_keyboardHintBoxMode);

    return keyboardWidget;
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
    lv_textarea_set_placeholder_text(ta, _("change_passcode_mid_btn"));
    lv_obj_set_size(ta, 352, 100);
    lv_obj_align(ta, LV_ALIGN_DEFAULT, 36, 332);
    lv_obj_set_style_text_opa(ta, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ta, DARK_BG_COLOR, LV_PART_MAIN);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_max_length(ta, PASSWORD_MAX_LEN);
    lv_textarea_set_one_line(ta, true);

    keyboardWidget->kb = kb;

    img = GuiCreateImg(keyboardHintBox, &imgEyeOff);
    lv_obj_align_to(img, ta, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, ta);
    keyboardWidget->eyeImg = img;

    button = GuiCreateImgLabelButton(keyboardHintBox, _("FORGET"), &imgLock, 124, ForgetHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 333, 439);

    button = GuiCreateImgLabelButton(keyboardHintBox, _("password_label"), &imgSwitch, 156, PassWordPinSwitchHandler, keyboardWidget);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 439);
    keyboardWidget->switchLabel = lv_obj_get_child(button, 1);

    label = GuiCreateIllustrateLabel(keyboardHintBox, _("password_error_not_match"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 390);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_recolor(label, true);
    keyboardWidget->errLabel = label;

    for (int i = 0; i < CREATE_PIN_NUM; i++) {
        lv_obj_t *led = GuiCreateLed(keyboardHintBox);
        lv_obj_align(led, LV_ALIGN_DEFAULT, 36 + 30 * i, 344);
        keyboardWidget->led[i] = led;
    }
    keyboardWidget->currentNum = 0;

    lv_obj_t *btnm = GuiCreateNumKeyboard(keyboardHintBox, SetPinEventHandler, NUM_KEYBOARD_PIN, keyboardWidget);
    lv_obj_add_style(btnm, &g_enterPressBtnmStyle, LV_STATE_PRESSED | LV_PART_ITEMS);
    lv_obj_align(btnm, LV_ALIGN_DEFAULT, 0, 490);
    keyboardWidget->btnm = btnm;

    PassWordPinHintSwitch(keyboardWidget, g_keyboardHintBoxMode);
    return keyboardWidget;
}

void GuiDeleteKeyboardWidget(KeyboardWidget_t *keyboardWidget)
{
    if (keyboardWidget != NULL && keyboardWidget->self != NULL) {
        memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
        keyboardWidget->currentNum = 0;
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
    if (keyboardWidget != NULL && keyboardWidget->kb != NULL && keyboardWidget->kb->ta != NULL && lv_obj_is_valid(keyboardWidget->kb->ta)) {
        lv_textarea_set_text(keyboardWidget->kb->ta, "");
    }
}

void GuiSetErrorLabel(KeyboardWidget_t *keyboardWidget, char *errorMessage)
{
    if (keyboardWidget != NULL && keyboardWidget->errLabel != NULL && lv_obj_is_valid(keyboardWidget->errLabel)) {
        lv_label_set_text(keyboardWidget->errLabel, errorMessage);
        lv_obj_clear_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiShowErrorLabel(KeyboardWidget_t *keyboardWidget)
{
    if (keyboardWidget != NULL && keyboardWidget->errLabel != NULL && lv_obj_is_valid(keyboardWidget->errLabel) && lv_obj_has_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiHideErrorLabel(KeyboardWidget_t *keyboardWidget)
{
    if (keyboardWidget != NULL && !lv_obj_has_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(keyboardWidget->errLabel, LV_OBJ_FLAG_HIDDEN);
    }
}


void GuiShowErrorNumber(KeyboardWidget_t *keyboardWidget, PasswordVerifyResult_t *passwordVerifyResult)
{
    memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
    keyboardWidget->currentNum = 0;
    printf("GuiShowErrorNumber error count is %d\n", passwordVerifyResult->errorCount);
    char hint[BUFFER_SIZE_128];
    char tempBuf[BUFFER_SIZE_128];
    uint8_t cnt = MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX - passwordVerifyResult->errorCount;
    if (cnt > 1) {
        snprintf_s(hint, BUFFER_SIZE_128, _("unlock_device_attempts_left_plural_times_fmt"), cnt);
    } else {
        snprintf_s(hint, BUFFER_SIZE_128, _("unlock_device_attempts_left_singular_times_fmt"), cnt);
    }
    snprintf_s(tempBuf, BUFFER_SIZE_128, "#F55831 %s#", hint);
    GuiSetErrorLabel(keyboardWidget, tempBuf);
    if (passwordVerifyResult->errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
        CloseUsb();
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

    char buf[BUFFER_SIZE_32] = {0};
    --(*keyboardWidget->timerCounter);
    if (*keyboardWidget->timerCounter > 0) {
        snprintf_s(buf, BUFFER_SIZE_32, _("unlock_device_error_btn_text_fmt"), *keyboardWidget->timerCounter);
    } else {
        strcpy_s(buf, BUFFER_SIZE_32, _("unlock_device_error_btn_end_text"));
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
