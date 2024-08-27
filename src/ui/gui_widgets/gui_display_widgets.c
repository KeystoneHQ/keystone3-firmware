#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_display_widgets.h"
#include "device_setting.h"
#include "gui_page.h"
#include "gui_api.h"

typedef enum {
    FIFTEEN_SECONDS = 0,
    THIRTY_SECONDS,
    ONE_MINUTE,
    FIVE_MINUTE,
    TEN_MINUTE,

    AUTO_LOCK_TIME_BUTT,
} AUTO_LOCK_TIME_ENUM;

typedef enum {
    ONE_HOUR = 0,
    SIX_HOUR,
    TWELVE_HOUR,
    ONE_DAY,
    NEVER,

    AUTO_SHUTDOWN_BUTT,
} AUTO_SHUTDOWN_ENUM;

static lv_obj_t *g_cont;
static lv_obj_t *autoLockAutoLockLabel;
static lv_obj_t *g_noticeWindow = NULL;
static lv_obj_t *g_autoLockCheck[AUTO_LOCK_TIME_BUTT];
static uint32_t g_currentAutoLockIndex = 2;

static lv_obj_t *autoShutDownTimeLabel;
static lv_obj_t *g_autoShutdownCheck[AUTO_SHUTDOWN_BUTT];
static uint32_t g_currentShutdownIndex = 2;

static lv_timer_t *g_delayTaskTimer;
static PageWidget_t *g_pageWidget;

static void GuiDisplayNVSBarInit(void);
static void GuiDisplayEntranceWidget(lv_obj_t *parent);

static void SliderEventCb(lv_event_t * e);
static void SetLowestBrightness(lv_event_t *e);
static void SetHighestBrightness(lv_event_t *e);
static void SaveSystemSetting(lv_timer_t *timer);
static lv_obj_t* GuiCreateSlider(lv_obj_t *parent);

static void ChooseAutoLockTimeHandler(lv_event_t *e);
static void CloseChooseAutoLockTimeHandler(lv_event_t* e);
static void SelectAutoLockTimeHandler(lv_event_t *e);
static uint32_t GetAutoLockTimeByEnum(AUTO_LOCK_TIME_ENUM lock_time_enum);
static const char *GetAutoLockTimeDescByLockTime();

static void ChooseAutoShutdownHandler(lv_event_t *e);
static void CloseChooseAutoShutdownHandler(lv_event_t* e);
static void SelectAutoShutdownHandler(lv_event_t *e);
static uint32_t GetAutoShutdownTimeByEnum(AUTO_SHUTDOWN_ENUM shutdownTime);
static const char *GetAutoShutdownTimeDescByLockTime(void);
static void NftScreenSaverSwitchHandler(lv_event_t * e);
static void OpenNftTutorialHandler(lv_event_t *e);

void GuiDisplayWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiDisplayEntranceWidget(cont);
}

void GuiDisplayWidgetsDeInit()
{
    GUI_DEL_OBJ(g_noticeWindow);
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiDisplayWidgetsRefresh()
{
    GuiDisplayNVSBarInit();
}

void GuiDisplayWidgetsRestart()
{}

static void GuiDisplayNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("system_settings_screen_lock_title"));
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}

static lv_obj_t* GuiCreateSlider(lv_obj_t *parent)
{

    lv_obj_t *slider = lv_slider_create(parent);
    lv_obj_set_size(slider, 288, 10);

    lv_obj_set_style_bg_color(slider, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(slider, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_border_color(slider, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(slider, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_border_width(slider, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(slider, 0, LV_PART_MAIN);

    static lv_style_t style_indicator;
    lv_style_init(&style_indicator);
    lv_style_set_bg_color(&style_indicator, lv_color_hex(0xf5870a));
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);

    static lv_style_t style_knob;
    lv_style_init(&style_knob);
    lv_style_set_bg_color(&style_knob, WHITE_COLOR);
    lv_style_set_width(&style_knob, 32);
    lv_style_set_height(&style_knob, 32);
    lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);

    return slider;
}

void GuiDisplayEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTextLabel(parent, _("system_settings_screen_lock_brightness"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 36);

    lv_obj_t *leftContainer = GuiCreateContainerWithParent(parent, 64, 64);
    lv_obj_t *leftImg = GuiCreateImg(leftContainer, &imgBrightnessLow);
    lv_obj_align(leftImg, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *rightContainer = GuiCreateContainerWithParent(parent, 64, 64);
    lv_obj_t *rightImg = GuiCreateImg(rightContainer, &imgBrightnessHigh);
    lv_obj_align(rightImg, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *slider = GuiCreateSlider(parent);
    int brightness = (GetBright() - MIN_BRIGHT) * 100 / (MAX_BRIGHT - MIN_BRIGHT);
    if (brightness < 0) {
        brightness = 0;
    } else if (brightness > 100) {
        brightness = 100;
    }
    lv_slider_set_value(slider, brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, SliderEventCb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_flag(leftContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(leftContainer, SetLowestBrightness, LV_EVENT_CLICKED, slider);
    lv_obj_set_style_bg_opa(leftContainer, LV_OPA_0, 0);
    lv_obj_set_style_bg_color(leftContainer, WHITE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leftContainer, LV_OPA_10 + LV_OPA_2, LV_STATE_PRESSED | LV_PART_MAIN);

    lv_obj_add_flag(rightContainer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(rightContainer, SetHighestBrightness, LV_EVENT_CLICKED, slider);
    lv_obj_set_style_bg_opa(rightContainer, LV_OPA_0, 0);
    lv_obj_set_style_bg_color(rightContainer, WHITE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rightContainer, LV_OPA_10 + LV_OPA_2, LV_STATE_PRESSED | LV_PART_MAIN);

    GuiButton_t brightnessTable[] = {
        {.obj = leftContainer, .align = LV_ALIGN_DEFAULT, .position = {10, 0}},
        {.obj = rightContainer, .align = LV_ALIGN_DEFAULT, .position = {382, 0}},
        {.obj = slider, .align = LV_ALIGN_CENTER, .position = {0, 0}}
    };

    lv_obj_t *button = GuiCreateButton(parent, 456, 64, brightnessTable, NUMBER_OF_ARRAYS(brightnessTable),
                                       UnHandler, NULL);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 84);

    label = GuiCreateTextLabel(parent, _("nft_screen_saver"));
    lv_obj_t *img = GuiCreateImg(parent, &imgInfo);
    GuiButton_t nftTable[] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {0, 0}},
        {.obj = img, .align = LV_ALIGN_LEFT_MID, .position = {16 + lv_obj_get_self_width(label), 0},},
    };
    lv_obj_t *btn = GuiCreateButton(parent, lv_obj_get_self_width(label) + lv_obj_get_self_width(img) + 16, 48, nftTable, NUMBER_OF_ARRAYS(nftTable),
                                    OpenNftTutorialHandler, NULL);
    lv_obj_set_style_radius(btn, 12, LV_PART_MAIN);
    lv_obj_t *nftSwitch = GuiCreateSwitch(parent);
    if (!IsNftScreenValid()) {
        lv_obj_set_style_bg_opa(nftSwitch, LV_OPA_30, LV_PART_KNOB);
    } else {
        if (GetNftScreenSaver()) {
            lv_obj_add_state(nftSwitch, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(nftSwitch, LV_STATE_CHECKED);
        }
    }
    lv_obj_clear_flag(nftSwitch, LV_OBJ_FLAG_CLICKABLE);
    nftTable[0].obj = btn;
    nftTable[0].position.x = 24;
    nftTable[1].obj = nftSwitch;
    nftTable[1].align = LV_ALIGN_RIGHT_MID;
    nftTable[1].position.x = -24;
    button = GuiCreateButton(parent, 456, 84, nftTable, NUMBER_OF_ARRAYS(nftTable),
                             NftScreenSaverSwitchHandler, nftSwitch);
    GuiAlignToPrevObj(button, LV_ALIGN_OUT_BOTTOM_MID, 0, 25);

    label = GuiCreateTextLabel(parent, _("system_settings_screen_lock_auto_lock"));
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);
    const char *currentLockTime = GetAutoLockTimeDescByLockTime();
    autoLockAutoLockLabel = GuiCreateNoticeLabel(parent, currentLockTime);

    GuiButton_t table[] = {
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {24, 24}},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {396, 24}},
        {.obj = autoLockAutoLockLabel, .align = LV_ALIGN_DEFAULT, .position = {24, 64}},
    };
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                             ChooseAutoLockTimeHandler, NULL);

    GuiAlignToPrevObj(button, LV_ALIGN_OUT_BOTTOM_MID, 0, 25);

    label = GuiCreateTextLabel(parent, _("auto_shutdown"));

    const char *currentShutdownTime = GetAutoShutdownTimeDescByLockTime();
    autoShutDownTimeLabel = GuiCreateNoticeLabel(parent, currentShutdownTime);
    imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = label;
    table[1].obj = imgArrow;
    table[2].obj = autoShutDownTimeLabel;

    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                             ChooseAutoShutdownHandler, NULL);

    GuiAlignToPrevObj(button, LV_ALIGN_OUT_BOTTOM_MID, 0, 25);
}

static void SetLowestBrightness(lv_event_t *e)
{
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_user_data(e);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
    lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
}

static void SetHighestBrightness(lv_event_t *e)
{
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_user_data(e);
    lv_slider_set_value(slider, 100, LV_ANIM_OFF);
    lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);

}

static void SliderEventCb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int brightness = MIN_BRIGHT + (MAX_BRIGHT - MIN_BRIGHT) * ((float)lv_slider_get_value(slider) / 100);
    SetBright(brightness);
    if (g_delayTaskTimer != NULL) {
        lv_timer_del(g_delayTaskTimer);
        g_delayTaskTimer = NULL;
        UNUSED(g_delayTaskTimer);
    }
    g_delayTaskTimer = lv_timer_create(SaveSystemSetting, 200, NULL);
}

static void SaveSystemSetting(lv_timer_t *timer)
{
    SaveDeviceSettings();
    lv_timer_del(timer);
    g_delayTaskTimer = NULL;
    UNUSED(g_delayTaskTimer);
}

static void ChooseAutoLockTimeHandler(lv_event_t *e)
{
    g_noticeWindow = GuiCreateHintBox(570);

    lv_obj_add_event_cb(lv_obj_get_child(g_noticeWindow, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *label = GuiCreateIllustrateLabel(g_noticeWindow, _("system_settings_screen_lock_auto_lock_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 260);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgClose);
    GuiButton_t tableHintbox = {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {0, 0} };
    lv_obj_t *buttonClose = GuiCreateButton(g_noticeWindow, 36, 36, &tableHintbox, 1, CloseChooseAutoLockTimeHandler, &g_noticeWindow);
    lv_obj_align(buttonClose, LV_ALIGN_DEFAULT, 408, 257);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_lock_15secs"));
    lv_obj_t *checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    GetAutoLockTimeDescByLockTime();
    if (g_currentAutoLockIndex == 0) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }

    g_autoLockCheck[0] = checkBox;
    GuiButton_t tables[2] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
        {.obj = checkBox, .align = LV_ALIGN_TOP_MID, .position = {0, 24},},
    };

    lv_obj_t *button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[0]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 320);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_lock_30secs"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentAutoLockIndex == 1) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoLockCheck[1] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[1]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 416);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_lock_1min"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentAutoLockIndex == 2) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoLockCheck[2] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[2]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 512);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_lock_5mins"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentAutoLockIndex == 3) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoLockCheck[3] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[3]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 608);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_lock_10mins"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentAutoLockIndex == 4) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoLockCheck[4] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[4]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 704);
}

static void CloseChooseAutoLockTimeHandler(lv_event_t* e)
{
    GUI_DEL_OBJ(g_noticeWindow)
}

static void SelectAutoLockTimeHandler(lv_event_t *e)
{
    for (int i = 0; i < AUTO_LOCK_TIME_BUTT; i++) {
        lv_obj_clear_state(g_autoLockCheck[i], LV_STATE_CHECKED);
    }

    int newCheckIndex = 0;
    lv_obj_t *newCheckBox = lv_event_get_user_data(e);
    for (int i = FIFTEEN_SECONDS; i < AUTO_LOCK_TIME_BUTT; i++) {
        if (newCheckBox == g_autoLockCheck[i]) {
            newCheckIndex = i;
            break;
        }
    }
    lv_obj_add_state(newCheckBox, LV_STATE_CHECKED);
    if (g_currentAutoLockIndex != newCheckIndex) {
        uint32_t lockTime = GetAutoLockTimeByEnum(newCheckIndex);
        SetAutoLockScreen(lockTime);
        SaveDeviceSettings();
        const char *currentLockTime = GetAutoLockTimeDescByLockTime();
        lv_label_set_text(autoLockAutoLockLabel, currentLockTime);
        GUI_DEL_OBJ(g_noticeWindow)
    }
}

static uint32_t GetAutoLockTimeByEnum(AUTO_LOCK_TIME_ENUM lock_time_enum)
{
    switch (lock_time_enum) {
    case FIFTEEN_SECONDS:
        return 15;
    case THIRTY_SECONDS:
        return 30;
    case ONE_MINUTE:
        return 60;
    case FIVE_MINUTE:
        return 60 * 5;
    case TEN_MINUTE:
        return 60 * 10;
    default:
        break;
    }
    return 0;
}

static const char *GetAutoLockTimeDescByLockTime()
{
    uint32_t currentAutoLockTime = GetAutoLockScreen();

    switch (currentAutoLockTime) {
    case 15:
        g_currentAutoLockIndex = 0;
        return _("system_settings_screen_lock_auto_lock_15secs");
    case 30:
        g_currentAutoLockIndex = 1;
        return _("system_settings_screen_lock_auto_lock_30secs");
    case 60:
        g_currentAutoLockIndex = 2;
        return _("system_settings_screen_lock_auto_lock_1min");
    case 300:
        g_currentAutoLockIndex = 3;
        return _("system_settings_screen_lock_auto_lock_5mins");
    case 600:
        g_currentAutoLockIndex = 4;
        return _("system_settings_screen_lock_auto_lock_10mins");
    default:
        break;
    }
    return "";
}

static void ChooseAutoShutdownHandler(lv_event_t *e)
{
    g_noticeWindow = GuiCreateHintBox(570);

    lv_obj_add_event_cb(lv_obj_get_child(g_noticeWindow, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *label = GuiCreateIllustrateLabel(g_noticeWindow, _("auto_shutdown_20"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 260);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgClose);
    GuiButton_t tableHintbox = {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {0, 0} };
    lv_obj_t *buttonClose = GuiCreateButton(g_noticeWindow, 36, 36, &tableHintbox, 1, CloseChooseAutoShutdownHandler, &g_noticeWindow);
    lv_obj_align(buttonClose, LV_ALIGN_DEFAULT, 408, 257);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_power_off_1h"));
    lv_obj_t *checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    GetAutoShutdownTimeDescByLockTime();
    if (g_currentShutdownIndex == 0) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoShutdownCheck[0] = checkBox;
    GuiButton_t tables[2] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
        {.obj = checkBox, .align = LV_ALIGN_TOP_MID, .position = {0, 24},},
    };

    lv_obj_t *button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[0]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 320);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_power_off_6h"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentShutdownIndex == 1) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoShutdownCheck[1] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[1]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 416);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_power_off_12h"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentShutdownIndex == 2) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoShutdownCheck[2] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[2]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 512);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_power_off_1d"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentShutdownIndex == 3) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoShutdownCheck[3] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[3]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 608);

    label = GuiCreateTextLabel(g_noticeWindow, _("system_settings_screen_lock_auto_power_off_never"));
    checkBox = GuiCreateSingleCheckBox(g_noticeWindow, "");
    if (g_currentShutdownIndex == 4) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    }
    g_autoShutdownCheck[4] = checkBox;
    tables[0].obj = label;
    tables[1].obj = checkBox;
    button = GuiCreateButton(g_noticeWindow, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[4]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 704);
}

static void CloseChooseAutoShutdownHandler(lv_event_t* e)
{
    GUI_DEL_OBJ(g_noticeWindow)
}

static void SelectAutoShutdownHandler(lv_event_t *e)
{
    for (int i = 0; i < AUTO_SHUTDOWN_BUTT; i++) {
        lv_obj_clear_state(g_autoShutdownCheck[i], LV_STATE_CHECKED);
    }
    int newCheckIndex = 0;
    lv_obj_t *newCheckBox = lv_event_get_user_data(e);
    for (int i = ONE_HOUR; i < AUTO_SHUTDOWN_BUTT; i++) {
        if (newCheckBox == g_autoShutdownCheck[i]) {
            newCheckIndex = i;
            break;
        }
    }
    lv_obj_add_state(newCheckBox, LV_STATE_CHECKED);
    if (g_currentShutdownIndex != newCheckIndex) {
        uint32_t powerOffTime = GetAutoShutdownTimeByEnum(newCheckIndex);
        SetAutoPowerOff(powerOffTime);
        SaveDeviceSettings();
        const char *currentPowerOffTime = GetAutoShutdownTimeDescByLockTime();
        lv_label_set_text(autoShutDownTimeLabel, currentPowerOffTime);
        GUI_DEL_OBJ(g_noticeWindow)
    }
}

static uint32_t GetAutoShutdownTimeByEnum(AUTO_SHUTDOWN_ENUM shutdownTime)
{
    switch (shutdownTime) {
    case ONE_HOUR:
        return 1;
    case SIX_HOUR:
        return 6;
    case TWELVE_HOUR:
        return 12;
    case ONE_DAY:
        return 24;
    case NEVER:
        return 0;
    default:
        break;
    }
    return 0;
}

static const char *GetAutoShutdownTimeDescByLockTime(void)
{
    uint32_t currentPowerOff = GetAutoPowerOff();

    switch (currentPowerOff) {
    case 1:
        g_currentShutdownIndex = 0;
        return _("system_settings_screen_lock_auto_power_off_1h");
    case 6:
        g_currentShutdownIndex = 1;
        return _("system_settings_screen_lock_auto_power_off_6h");
    case 12:
        g_currentShutdownIndex = 2;
        return _("system_settings_screen_lock_auto_power_off_12h");
    case 24:
        g_currentShutdownIndex = 3;
        return _("system_settings_screen_lock_auto_power_off_1d");
    case 0:
        g_currentShutdownIndex = 4;
        return _("system_settings_screen_lock_auto_power_off_never");
    default:
        break;
    }
    return _("system_settings_screen_lock_auto_power_off_never");
}

static void NftScreenSaverSwitchHandler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_user_data(e);
    if (!IsNftScreenValid()) {
        return;
    }

    if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        lv_obj_clear_state(obj, LV_STATE_CHECKED);
        SetNftScreenSaver(false);
    } else {
        lv_obj_add_state(obj, LV_STATE_CHECKED);
        SetNftScreenSaver(true);
    }
    SaveDeviceSettings();
}

static void OpenNftTutorialHandler(lv_event_t *e)
{
    g_noticeWindow = GuiCreateHintBox(704);

    lv_obj_t *qrCodeCont = lv_obj_create(g_noticeWindow);
    lv_obj_set_size(qrCodeCont, 408, 408);
    lv_obj_set_style_border_width(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(qrCodeCont, 0, 0);
    lv_obj_set_style_pad_all(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(qrCodeCont, 16, LV_PART_MAIN);
    lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(qrCodeCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(qrCodeCont, LV_ALIGN_BOTTOM_MID, 0, -260);

    lv_obj_t *qrCode = lv_qrcode_create(qrCodeCont, 360, BLACK_COLOR, WHITE_COLOR);
    lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);
    lv_qrcode_update(qrCode, _("nft_screen_set_url"), (uint32_t)strnlen_s(_("nft_screen_set_url"), BUFFER_SIZE_128));

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("nft_screen_set_title"));
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -156);
    label = GuiCreateIllustrateLabel(g_noticeWindow, _("nft_screen_set_url"));
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -114);

    lv_obj_t *button = GuiCreateTextBtn(g_noticeWindow, _("OK"));
    lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(button, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}