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
#include "gui_keyboard_hintbox.h"
#ifdef COMPILE_SIMULATOR
#else
#include "drv_battery.h"
#endif

static lv_obj_t *container;
static lv_obj_t *vibrationSw;

static KeyboardWidget_t *g_keyboardWidget = NULL;

void GuiSystemSettingNVSBarInit();
void GuiSystemSettingEntranceWidget(lv_obj_t *parent);
static void GuiSystemSettingWipeDeivceHandler(lv_event_t *e);
static void GuiShowKeyBoardDialog(lv_obj_t *parent);
static void DispalyHandler(lv_event_t *e);

static void VibrationHandler(lv_event_t *e);
static void VibrationSwitchHandler(lv_event_t * e);

void OpenForgetPasswordHandler(lv_event_t *e);

void GuiSystemSettingAreaInit()
{
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
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    if (container != NULL) {
        lv_obj_del(container);
        container = NULL;
    }
}

void GuiSystemSettingAreaRefresh()
{
    GuiSystemSettingNVSBarInit();
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
    g_keyboardWidget = GuiCreateKeyboardWidget(parent);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
}


void GuiSystemSettingVerifyPasswordSuccess(void)
{
    printf("password is right\n");
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GuiFrameOpenView(&g_wipeDeviceView);
}

void GuiSystemSettingVerifyPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
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