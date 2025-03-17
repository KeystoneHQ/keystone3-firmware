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

static lv_obj_t *g_container;
static lv_obj_t *g_vibrationSw;
static lv_obj_t *g_bootSecureSw;
static lv_obj_t *g_recoveryModeSw;

static KeyboardWidget_t *g_keyboardWidget = NULL;
static PageWidget_t *g_pageWidget;
static PageWidget_t *g_selectLanguagePage;
static lv_obj_t *g_noticeWindow = NULL;

void GuiSystemSettingNVSBarInit();
void GuiSystemSettingEntranceWidget(lv_obj_t *parent);
static void GuiSystemSettingWipeDeivceHandler(lv_event_t *e);
static void GuiShowKeyBoardDialog(lv_obj_t *parent);
static void DispalyHandler(lv_event_t *e);
static void VibrationHandler(lv_event_t *e);
static void VibrationSwitchHandler(lv_event_t * e);
void GuiCreateLanguageWidget(lv_obj_t *parent, uint16_t offset);
void OpenForgetPasswordHandler(lv_event_t *e);
static void OpenLanguageSelectHandler(lv_event_t *e);
static void BootSecureSwitchHandler(lv_event_t * e);
static void RecoveryModeSwitchHandler(lv_event_t * e);
#ifdef WEB3_VERSION
static void PermitSingSwitchHandler(lv_event_t * e);
static lv_obj_t *g_permitSw;
#endif

void GuiSystemSettingAreaInit(void)
{
    g_pageWidget = CreatePageWidget();
    g_container = g_pageWidget->contentZone;

    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(g_container, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(g_container, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(g_container, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_container, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    GuiSystemSettingEntranceWidget(g_container);
}

void GuiSystemSettingWebAuthHandler(lv_event_t *e)
{
    GuiWebAuthSetEntry(WEB_AUTH_ENTRY_SETTING);
    GuiFrameOpenView(&g_webAuthView);
}

void GuiSystemSettingEntranceWidget(lv_obj_t *parent)
{
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *label, *button;
    uint16_t offset = 0;

    button = GuiCreateSelectButton(parent, _("language_little_title"), &imgArrowRight,
                                   OpenLanguageSelectHandler, NULL, false);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;

    button = GuiCreateSelectButton(parent, _("system_settings_screen_lock_title"), &imgArrowRight,
                                   DispalyHandler, NULL, false);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;

    label = GuiCreateTextLabel(parent, _("system_settings_vabiration"));
    g_vibrationSw = lv_switch_create(parent);
    lv_obj_add_event_cb(g_vibrationSw, VibrationSwitchHandler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(g_vibrationSw, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_vibrationSw, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_vibrationSw, LV_OPA_30, LV_PART_MAIN);
    if (GetVibration()) {
        lv_obj_add_state(g_vibrationSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_vibrationSw, LV_STATE_CHECKED);
    }
    GuiButton_t tableSwitch[] = {
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {24, 24},},
        {.obj = g_vibrationSw, .align = LV_ALIGN_DEFAULT, .position = {372, 24},},
    };
    button = GuiCreateButton(parent, 456, 84, tableSwitch, NUMBER_OF_ARRAYS(tableSwitch),
                             VibrationHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;

#ifdef WEB3_VERSION
    // permit sign
    g_permitSw = lv_switch_create(parent);
    lv_obj_clear_flag(g_permitSw, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(g_permitSw, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_permitSw, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_permitSw, LV_OPA_30, LV_PART_MAIN);
    if (GetPermitSign()) {
        lv_obj_add_state(g_permitSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_permitSw, LV_STATE_CHECKED);
    }
    tableSwitch[0].obj = GuiCreateTextLabel(parent, _("system_settings_permit_switch"));
    tableSwitch[1].obj = g_permitSw;
    button = GuiCreateButton(parent, 456, 84, tableSwitch, NUMBER_OF_ARRAYS(tableSwitch),
                             PermitSingSwitchHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;
#endif

    // boot secure
    g_bootSecureSw = lv_switch_create(parent);
    lv_obj_clear_flag(g_bootSecureSw, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(g_bootSecureSw, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_bootSecureSw, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_bootSecureSw, LV_OPA_30, LV_PART_MAIN);

    if (GetBootSecureCheckFlag()) {
        lv_obj_add_state(g_bootSecureSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_bootSecureSw, LV_STATE_CHECKED);
    }
    tableSwitch[0].obj = GuiCreateTextLabel(parent, _("boot_secure_switch_text_title"));
    tableSwitch[1].obj = g_bootSecureSw;

    button = GuiCreateButton(parent, 456, 84, tableSwitch, NUMBER_OF_ARRAYS(tableSwitch),
                             BootSecureSwitchHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, offset);
    offset += 100;

    // recovery mode
    g_recoveryModeSw = lv_switch_create(parent);
    lv_obj_clear_flag(g_recoveryModeSw, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(g_recoveryModeSw, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_recoveryModeSw, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_recoveryModeSw, LV_OPA_30, LV_PART_MAIN);

    if (GetRecoveryModeSwitch()) {
        lv_obj_add_state(g_recoveryModeSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_recoveryModeSw, LV_STATE_CHECKED);
    }
    tableSwitch[0].obj = GuiCreateTextLabel(parent, _("recovery_mode_switch_text_title"));
    tableSwitch[1].obj = g_recoveryModeSw;

    button = GuiCreateButton(parent, 456, 84, tableSwitch, NUMBER_OF_ARRAYS(tableSwitch),
                             RecoveryModeSwitchHandler, NULL);
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
    GUI_DEL_OBJ(g_noticeWindow)
    if (g_selectLanguagePage != NULL) {
        DestroyPageWidget(g_selectLanguagePage);
        g_selectLanguagePage = NULL;
    }

    GuiDeleteKeyboardWidget(g_keyboardWidget);
    if (g_container != NULL) {
        lv_obj_del(g_container);
        g_container = NULL;
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

static void GuiSystemSettingAreaRestartHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    uint8_t index = *(uint8_t *)lv_event_get_user_data(e);
    LanguageSwitch(index);
    if (g_selectLanguagePage != NULL) {
        DestroyPageWidget(g_selectLanguagePage);
        g_selectLanguagePage = NULL;
    }
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    GuiEnterPassLabelRefresh();
    GuiEmitSignal(GUI_EVENT_CHANGE_LANGUAGE, NULL, 0);
}

static void CloseChangeLanguageHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    LanguageSwitchTemp(LanguageGetIndex());
}

void GuiSystemSettingLanguage(void *param)
{
    LanguageSwitchTemp(*(uint8_t *)param);
    g_noticeWindow = GuiCreateGeneralHintBox(&imgBlueInformation, _("confirm_language_title"), _("confirm_language_desc"), NULL,
                     _("not_now"), WHITE_COLOR_OPA20, _("Confirm"), ORANGE_COLOR);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(leftBtn, CloseChangeLanguageHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(rightBtn, GuiSystemSettingAreaRestartHandler, LV_EVENT_CLICKED, param);
}

static void GuiSystemSettingWipeDeivceHandler(lv_event_t *e)
{
    if (GetCurrentDisplayPercent() < LOW_BATTERY_PERCENT) {
        GuiApiEmitSignalWithValue(SIG_INIT_LOW_BATTERY, 1);
    } else {
        GuiShowKeyBoardDialog(g_container);
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
    GuiFrameOpenView(&g_displayView);
}

static void VibrationHandler(lv_event_t *e)
{
    if (lv_obj_has_state(g_vibrationSw, LV_STATE_CHECKED)) {
        lv_obj_clear_state(g_vibrationSw, LV_STATE_CHECKED);
    } else {
        lv_obj_add_state(g_vibrationSw, LV_STATE_CHECKED);
    }
    lv_event_send(g_vibrationSw, LV_EVENT_VALUE_CHANGED, NULL);
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


static void DestroyLanguagePageWidgetHandler(lv_event_t *e)
{
    DestroyPageWidget(g_selectLanguagePage);
    g_selectLanguagePage = NULL;
}

static void OpenLanguageSelectHandler(lv_event_t *e)
{
    g_selectLanguagePage = CreatePageWidget();
    lv_obj_clear_flag(g_selectLanguagePage->contentZone, LV_OBJ_FLAG_SCROLLABLE);
    GuiCreateLanguageWidget(g_selectLanguagePage->contentZone, 12);
    SetNavBarLeftBtn(g_selectLanguagePage->navBarWidget, NVS_BAR_RETURN, DestroyLanguagePageWidgetHandler, g_selectLanguagePage);
    SetMidBtnLabel(g_selectLanguagePage->navBarWidget, NVS_BAR_MID_LABEL, _("language_title"));
}

static void GuiShowChangeKeyBoard(lv_event_t * e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    g_keyboardWidget = GuiCreateKeyboardWidget(g_container);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    uint16_t *sig = (uint16_t *)lv_event_get_user_data(e);
    SetKeyboardWidgetSig(g_keyboardWidget, sig);
}

void GuiDealBootSecureParamKeyBoard(uint16_t sig, bool pass)
{
    if (pass) {
        GUI_DEL_OBJ(g_noticeWindow)
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        if (sig == SIG_SETTING_CHANGE_BOOT_SECURE_SWITCH) {
            if (lv_obj_has_state(g_bootSecureSw, LV_STATE_CHECKED)) {
                lv_obj_clear_state(g_bootSecureSw, LV_STATE_CHECKED);
                SetBootSecureCheckFlag(false);
            } else {
                lv_obj_add_state(g_bootSecureSw, LV_STATE_CHECKED);
                SetBootSecureCheckFlag(true);
            }
        } else if (sig == SIG_SETTING_CHANGE_RECOVERY_MODE_SWITCH) {
            if (lv_obj_has_state(g_recoveryModeSw, LV_STATE_CHECKED)) {
                lv_obj_clear_state(g_recoveryModeSw, LV_STATE_CHECKED);
                SetRecoveryModeSwitch(false);
            } else {
                lv_obj_add_state(g_recoveryModeSw, LV_STATE_CHECKED);
                SetRecoveryModeSwitch(true);
            }
        }
    }
}

static void BootSecureSwitchHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    g_noticeWindow = GuiCreateGeneralHintBox(&imgWarn, _("boot_secure_switch_title"), _("boot_secure_switch_desc"), NULL,
                     _("Cancel"), WHITE_COLOR_OPA20, _("Change"), DEEP_ORANGE_COLOR);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
    static uint16_t sig = SIG_SETTING_CHANGE_BOOT_SECURE_SWITCH;
    lv_obj_add_event_cb(rightBtn, GuiShowChangeKeyBoard, LV_EVENT_CLICKED, &sig);
}

static void RecoveryModeSwitchHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    g_noticeWindow = GuiCreateGeneralHintBox(&imgWarn, _("recovery_mode_switch_title"), _("recovery_mode_switch_desc"), NULL,
                     _("Cancel"), WHITE_COLOR_OPA20, _("Change"), DEEP_ORANGE_COLOR);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
    static uint16_t sig = SIG_SETTING_CHANGE_RECOVERY_MODE_SWITCH;
    lv_obj_add_event_cb(rightBtn, GuiShowChangeKeyBoard, LV_EVENT_CLICKED, &sig);
}

#ifdef WEB3_VERSION
static void GuiShowChangePermitKeyBoard(lv_event_t * e);
void GuiDealChangePermitKeyBoard(bool pass)
{
    if (pass) {
        GUI_DEL_OBJ(g_noticeWindow)
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        if (lv_obj_has_state(g_permitSw, LV_STATE_CHECKED)) {
            lv_obj_clear_state(g_permitSw, LV_STATE_CHECKED);
            SetPermitSign(0);
        } else {
            lv_obj_add_state(g_permitSw, LV_STATE_CHECKED);
            SetPermitSign(1);
        }
        SaveDeviceSettings();
    }
}

static void PermitSingSwitchHandler(lv_event_t * e)
{
    g_noticeWindow = GuiCreateGeneralHintBox(&imgWarn, _("permit_switch_title"), _("permit_switch_desc"), NULL,
                     _("Cancel"), WHITE_COLOR_OPA20, _("Change"), DEEP_ORANGE_COLOR);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(rightBtn, GuiShowChangePermitKeyBoard, LV_EVENT_CLICKED, NULL);
}

static void GuiShowChangePermitKeyBoard(lv_event_t * e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    g_keyboardWidget = GuiCreateKeyboardWidget(g_container);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_SETTING_CHANGE_PERMIT_SWITCH;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}
#endif