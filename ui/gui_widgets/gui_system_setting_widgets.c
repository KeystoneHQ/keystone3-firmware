#include "gui.h"
#include "gui_views.h"
#include "gui_api.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_qrcode_widgets.h"
#include "gui_web_auth_widgets.h"
#include "gui_setting_widgets.h"
#include "gui_enter_passcode.h"
#include "secret_cache.h"
#include "device_setting.h"
#include "motor_manager.h"
#include "gui_lock_widgets.h"
#ifdef COMPILE_SIMULATOR
#else
#include "drv_battery.h"
#endif
static lv_obj_t *container;
typedef struct KeyBoardBundle {
    lv_obj_t *errLabel;
    KeyBoard_t *kb;
} KeyBoardBundle_t;

static KeyBoardBundle_t g_keyBoardBundle;

static lv_obj_t *g_noticeHintBox = NULL;
static lv_obj_t *g_errorHintBox = NULL;
static lv_timer_t *g_countDownTimer;
static int8_t countDown = 5;

static lv_obj_t *vibrationSw;

void GuiSystemSettingNVSBarInit();
void GuiSystemSettingEntranceWidget(lv_obj_t *parent);
static void GuiSystemSettingWipeDeivceHandler(lv_event_t *e);
static void GuiShowKeyBoardDialog(lv_obj_t *parent);
static void VerifyPasswordHandler(lv_event_t *e);
static void UnlockDeviceHandler(lv_event_t *e);
static void GuiShowPasswordErrorHintBox(void);
static void CountDownTimerWipeDeviceHandler(lv_timer_t *timer);
static void GuiHintBoxToLockSreen(void);
static void GuiCountDownDestruct(void *obj, void *param);
static void DispalyHandler(lv_event_t *e);

static void VibrationHandler(lv_event_t *e);
static void VibrationSwitchHandler(lv_event_t * e);
static void ForgetHandler(lv_event_t *e);

void OpenForgetPasswordHandler(lv_event_t *e);

void GuiSystemSettingAreaInit()
{
    GuiSystemSettingNVSBarInit();
    if (container != NULL) {
        lv_obj_del(container);
        container = NULL;
    }
    container = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                   GUI_MAIN_AREA_OFFSET);
    lv_obj_align(container, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(container, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(container, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    GuiSystemSettingEntranceWidget(container);
}

void GuiSystemSettingWebAuthHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiWebAuthSetEntry(WEB_AUTH_ENTRY_SETTING);
        GuiFrameOpenView(&g_webAuthView);
    }
}

void GuiSystemSettingEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t *label, *imgArrow, *button;

    label = GuiCreateTextLabel(parent, "Display & Lock Screen");
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    GuiButton_t table[] = {
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {36, 24},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {396, 24},
        },
    };
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                             DispalyHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 0);

    label = GuiCreateTextLabel(parent, "Vibration");
    vibrationSw = lv_switch_create(parent);
    lv_obj_add_event_cb(vibrationSw, VibrationSwitchHandler, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(vibrationSw, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(vibrationSw, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(vibrationSw, LV_OPA_30, LV_PART_MAIN);
    if (GetVibration()) {
        lv_obj_add_state(vibrationSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(vibrationSw, LV_STATE_CHECKED);
    }
    GuiButton_t tableSwitch[] = {
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {36, 24},
        },
        {
            .obj = vibrationSw,
            .align = LV_ALIGN_DEFAULT,
            .position = {372, 24},
        },
    };
    button = GuiCreateButton(parent, 456, 84, tableSwitch, NUMBER_OF_ARRAYS(tableSwitch),
                             VibrationHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 109);


    label = GuiCreateTextLabel(parent, _("verify_title"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[1].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                             GuiSystemSettingWebAuthHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 205);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 301);

    label = GuiCreateTextLabel(parent, "Wipe Device");
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[1].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                             GuiSystemSettingWipeDeivceHandler, NULL);
    lv_obj_set_style_text_color(lv_obj_get_child(button, 0), lv_color_hex(0xf55831), LV_PART_MAIN);

    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 301);

}

void GuiSystemSettingNVSBarInit()
{
    GuiNvsBarSetLeftCb(NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, _("device_setting_system_setting_title"));
}

void GuiSystemSettingAreaDeInit()
{
    if (container != NULL) {
        GUI_DEL_OBJ(g_noticeHintBox)
        lv_obj_del(container);
        container = NULL;
    }
}

void GuiSystemSettingAreaRefresh()
{
    GuiSystemSettingAreaDeInit();
    GuiSystemSettingAreaInit();
}


static void GuiSystemSettingWipeDeivceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (GetBatterPercent() < LOW_BATTERY_PERCENT) {
            GuiApiEmitSignalWithValue(SIG_INIT_LOW_BATTERY, 1);
        } else {
            GuiShowKeyBoardDialog(container);
        }
    }
}



static void GuiShowKeyBoardDialog(lv_obj_t *parent)
{

    g_noticeHintBox = GuiCreateHintBox(parent, 480, 576, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    lv_obj_t *label = GuiCreateIllustrateLabel(g_noticeHintBox, _("Please Enter Passcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 254);

    lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgClose);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, NULL);
    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_CENTER,
            .position = {0, 0},
        },
    };
    lv_obj_t *button = GuiCreateButton(g_noticeHintBox, 36, 36, table, NUMBER_OF_ARRAYS(table),
                                       CloseHintBoxHandler, &g_noticeHintBox);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 408, 251);

    g_keyBoardBundle.kb = GuiCreateFullKeyBoard(g_noticeHintBox, VerifyPasswordHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_keyBoardBundle.kb, 0);
    lv_obj_t *ta = g_keyBoardBundle.kb->ta;
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

    button = GuiCreateImgLabelButton(g_noticeHintBox, _("FORGET"), &imgLock, ForgetHandler, &g_systemSettingView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 333, 439);

    label = GuiCreateIllustrateLabel(g_noticeHintBox, _("Password does not match"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 390);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_keyBoardBundle.errLabel = label;
    lv_label_set_recolor(g_keyBoardBundle.errLabel, true);

}

static void ForgetHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeHintBox)
        OpenForgetPasswordHandler(e);
    }
}

static void VerifyPasswordHandler(lv_event_t *e)
{
    static uint16_t passCodeType = ENTER_PASSCODE_VERIFY_PASSWORD;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        const char *currText = lv_textarea_get_text(g_keyBoardBundle.kb->ta);
        SecretCacheSetPassword((char *)currText);
        GuiModelVerifyAmountPassWord(&passCodeType);
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        Vibrate(SLIGHT);
        if (!lv_obj_has_flag(g_keyBoardBundle.errLabel, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(g_keyBoardBundle.errLabel, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void GuiSystemSettingVerifyPasswordResult(bool result)
{
    if (result) {
        printf(" password is right\n");
        GUI_DEL_OBJ(g_noticeHintBox)
        GuiFrameOpenView(&g_wipeDeviceView);
    } else {
        printf(" password is error\n");
        lv_obj_clear_flag(g_keyBoardBundle.errLabel, LV_OBJ_FLAG_HIDDEN);
        lv_textarea_set_text(g_keyBoardBundle.kb->ta, "");
    }
}

void GuiSystemSettingVerifyPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    printf("GuiSystemSettingVerifyPasswordErrorCount  errorcount is %d\n", passwordVerifyResult->errorCount);

    if (g_keyBoardBundle.errLabel != NULL) {
        char hint[128];
        sprintf(hint, "Incorrect password, you have #F55831 %d# chances left", (MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX - passwordVerifyResult->errorCount));
        lv_label_set_text(g_keyBoardBundle.errLabel, hint);
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

    if (!lv_obj_has_flag(g_keyBoardBundle.errLabel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(g_keyBoardBundle.errLabel, LV_OBJ_FLAG_HIDDEN);
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

static void DispalyHandler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenView(&g_displayView);
    }

}

static void VibrationHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_has_state(vibrationSw, LV_STATE_CHECKED)) {
            lv_obj_clear_state(vibrationSw, LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(vibrationSw, LV_STATE_CHECKED);
        }
        lv_event_send(vibrationSw, LV_EVENT_VALUE_CHANGED, NULL);
    }

}

static void VibrationSwitchHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
            printf("vibration on\n");
            SetVibration(1);
            UnlimitedVibrate(SLIGHT);
        } else {
            printf("vibration off\n");
            SetVibration(0);
        }
        SaveDeviceSettings();
    }
}