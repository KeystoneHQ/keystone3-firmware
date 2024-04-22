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
#include "gui_web_auth_widgets.h"
#include "gui_setting_widgets.h"
#include "gui_enter_passcode.h"
#include "secret_cache.h"
#include "device_setting.h"
#include "motor_manager.h"
#include "gui_lock_widgets.h"
#include "gui_keyboard_hintbox.h"
#include "gui_page.h"
#ifdef COMPILE_SIMULATOR
#else
#include "drv_battery.h"
#endif

static lv_obj_t *container;
static lv_obj_t *vibrationSw;

static KeyboardWidget_t *g_keyboardWidget = NULL;
static PageWidget_t *g_pageWidget;
static PageWidget_t *g_selectLanguagePage;

void GuiSystemSettingNVSBarInit();
void GuiSystemSettingEntranceWidget(lv_obj_t *parent);
static void GuiSystemSettingWipeDeivceHandler(lv_event_t *e);
static void GuiShowKeyBoardDialog(lv_obj_t *parent);
static void DispalyHandler(lv_event_t *e);
static void OpenLanguageSelectHandler(lv_event_t *e);
static void VibrationHandler(lv_event_t *e);
static void VibrationSwitchHandler(lv_event_t * e);
void GuiCreateLanguageWidget(lv_obj_t *parent, uint16_t offset);
void OpenForgetPasswordHandler(lv_event_t *e);

void GuiSystemSettingAreaInit(void)
{
    g_pageWidget = CreatePageWidget();
    container = g_pageWidget->contentZone;

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
    lv_obj_t *label, *button;
    uint16_t offset = 0;

#ifndef BTC_ONLY
    button = GuiCreateSelectButton(parent, _("language_little_title"), &imgArrowRight,
                                   OpenLanguageSelectHandler, NULL, false);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;
#endif

    button = GuiCreateSelectButton(parent, _("system_settings_screen_lock_title"), &imgArrowRight,
                                   DispalyHandler, NULL, false);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;

    label = GuiCreateTextLabel(parent, _("system_settings_vabiration"));
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
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {24, 24},},
        {.obj = vibrationSw, .align = LV_ALIGN_DEFAULT, .position = {372, 24},},
    };
    button = GuiCreateButton(parent, 456, 84, tableSwitch, NUMBER_OF_ARRAYS(tableSwitch),
                             VibrationHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;

    button = GuiCreateSelectButton(parent, _("verify_title_text"), &imgArrowRight,
                                   GuiSystemSettingWebAuthHandler, NULL, false);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 397);

    button = GuiCreateSelectButton(parent, _("wipe_device"), &imgArrowRight,
                                   GuiSystemSettingWipeDeivceHandler, NULL, false);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    lv_obj_set_style_text_color(lv_obj_get_child(button, 0), lv_color_hex(0xf55831), LV_PART_MAIN);
}

void GuiSystemSettingNVSBarInit(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("device_setting_system_setting_title"));
}

void GuiSystemSettingAreaDeInit(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    if (container != NULL) {
        lv_obj_del(container);
        container = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiSystemSettingAreaRefresh(void)
{
    GuiSystemSettingNVSBarInit();
    PassWordPinHintRefresh(g_keyboardWidget);
}

void GuiSystemSettingAreaRestart()
{
    if (g_selectLanguagePage != NULL) {
        DestroyPageWidget(g_selectLanguagePage);
        g_selectLanguagePage = NULL;
    }
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
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

static void OpenLanguageSelectHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        g_selectLanguagePage = CreatePageWidget();
        lv_obj_clear_flag(g_selectLanguagePage->contentZone, LV_OBJ_FLAG_SCROLLABLE);
        GuiCreateLanguageWidget(g_selectLanguagePage->contentZone, 12);
        SetNavBarLeftBtn(g_selectLanguagePage->navBarWidget, NVS_BAR_RETURN, DestroyPageWidgetHandler, g_selectLanguagePage);
        SetMidBtnLabel(g_selectLanguagePage->navBarWidget, NVS_BAR_MID_LABEL, _("language_title"));
    }
}
