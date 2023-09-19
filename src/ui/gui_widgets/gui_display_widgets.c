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
static lv_obj_t *g_autoLockHintBox = NULL;
static lv_obj_t *g_autoLockCheck[AUTO_LOCK_TIME_BUTT];
static uint32_t g_currentAutoLockIndex = 2;

static lv_obj_t *autoShutDownTimeLabel;
static lv_obj_t *g_autoShutdownHintBox = NULL;
static lv_obj_t *g_autoShutdownCheck[AUTO_SHUTDOWN_BUTT];
static uint32_t g_currentShutdownIndex = 2;

static lv_timer_t *g_delayTaskTimer;
static PageWidget_t *g_pageWidget;

static void GuiDisplayNVSBarInit(void);
static void GuiDisplayEntranceWidget(lv_obj_t *parent);

static void UnHandler(lv_event_t *e);
static void SliderEventCb(lv_event_t * e);
static void SetLowestBrightness(lv_event_t *e);
static void SetHighestBrightness(lv_event_t *e);
static void SaveSystemSetting(lv_timer_t *timer);
static lv_obj_t* GuiCreateSlider(lv_obj_t *parent);

static void ChooseAutoLockTimeHandler(lv_event_t *e);
static void CloseChooseAutoLockTimeHandler(lv_event_t* e);
static void SelectAutoLockTimeHandler(lv_event_t *e);
static uint32_t GetAutoLockTimeByEnum(AUTO_LOCK_TIME_ENUM lock_time_enum);
static char* GetAutoLockTimeDescByLockTime(void);

static void ChooseAutoShutdownHandler(lv_event_t *e);
static void CloseChooseAutoShutdownHandler(lv_event_t* e);
static void SelectAutoShutdownHandler(lv_event_t *e);
static uint32_t GetAutoShutdownTimeByEnum(AUTO_SHUTDOWN_ENUM shutdownTime);
static char* GetAutoShutdownTimeDescByLockTime(void);

void GuiDisplayWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiDisplayEntranceWidget(cont);
}

void GuiDisplayWidgetsDeInit()
{
    GUI_DEL_OBJ(g_autoLockHintBox);
    GUI_DEL_OBJ(g_autoShutdownHintBox);
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
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Display & Lock Screen");
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
    lv_obj_t *label = GuiCreateTextLabel(parent, "Brightness");
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
        {
            .obj = leftContainer,
            .align = LV_ALIGN_DEFAULT,
            .position = {10, 0},
        },
        {
            .obj = rightContainer,
            .align = LV_ALIGN_DEFAULT,
            .position = {382, 0},
        },
        {
            .obj = slider,
            .align = LV_ALIGN_CENTER,
            .position = {0, 0},
        },
    };

    lv_obj_t *button = GuiCreateButton(parent, 456, 64, brightnessTable, NUMBER_OF_ARRAYS(brightnessTable),
                                       UnHandler, NULL);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 84);

    label = GuiCreateTextLabel(parent, "Auto Lock");
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);
    char *currentLockTime = GetAutoLockTimeDescByLockTime();
    autoLockAutoLockLabel = GuiCreateNoticeLabel(parent, currentLockTime);

    GuiButton_t table[] = {
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 24},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {396, 24},
        },
        {
            .obj = autoLockAutoLockLabel,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 64},
        },
    };
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                             ChooseAutoLockTimeHandler, NULL);

    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 173);


    label = GuiCreateTextLabel(parent, "Auto Shutdown");

    char *currentShutdownTime = GetAutoShutdownTimeDescByLockTime();
    autoShutDownTimeLabel = GuiCreateNoticeLabel(parent, currentShutdownTime);
    imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = label;
    table[1].obj = imgArrow;
    table[2].obj = autoShutDownTimeLabel;

    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                             ChooseAutoShutdownHandler, NULL);

    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 316);


}

static void UnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}

static void SetLowestBrightness(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t * slider = (lv_obj_t *)lv_event_get_user_data(e);
        lv_slider_set_value(slider, 0, LV_ANIM_OFF);
        lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
    }

}

static void SetHighestBrightness(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t * slider = (lv_obj_t *)lv_event_get_user_data(e);
        lv_slider_set_value(slider, 100, LV_ANIM_OFF);
        lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
    }

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
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {

        g_autoLockHintBox = GuiCreateHintBox(lv_scr_act(), 480, 570, true);

        lv_obj_add_event_cb(lv_obj_get_child(g_autoLockHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_autoLockHintBox);

        lv_obj_t *label = GuiCreateIllustrateLabel(g_autoLockHintBox, "Timeout Duration");
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 260);
        lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

        lv_obj_t *img = GuiCreateImg(g_autoLockHintBox, &imgClose);
        GuiButton_t tableHintbox = {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {0, 0} };
        lv_obj_t *buttonClose = GuiCreateButton(g_autoLockHintBox, 36, 36, &tableHintbox, 1, CloseChooseAutoLockTimeHandler, &g_autoLockHintBox);
        lv_obj_align(buttonClose, LV_ALIGN_DEFAULT, 408, 257);

        label = GuiCreateTextLabel(g_autoLockHintBox, "15 seconds");
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(g_autoLockHintBox, "");
        GetAutoLockTimeDescByLockTime();
        if (g_currentAutoLockIndex == 0) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }

        g_autoLockCheck[0] = checkBox;
        GuiButton_t tables[2] = {
            {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
            {.obj = checkBox, .align = LV_ALIGN_TOP_MID, .position = {0, 24},},
        };

        lv_obj_t *button = GuiCreateButton(g_autoLockHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[0]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 320);


        label = GuiCreateTextLabel(g_autoLockHintBox, "30 seconds");
        checkBox = GuiCreateSingleCheckBox(g_autoLockHintBox, "");
        if (g_currentAutoLockIndex == 1) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoLockCheck[1] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoLockHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[1]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 416);

        label = GuiCreateTextLabel(g_autoLockHintBox, "1 minute");
        checkBox = GuiCreateSingleCheckBox(g_autoLockHintBox, "");
        if (g_currentAutoLockIndex == 2) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoLockCheck[2] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoLockHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[2]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 512);

        label = GuiCreateTextLabel(g_autoLockHintBox, "5 minutes");
        checkBox = GuiCreateSingleCheckBox(g_autoLockHintBox, "");
        if (g_currentAutoLockIndex == 3) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoLockCheck[3] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoLockHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[3]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 608);

        label = GuiCreateTextLabel(g_autoLockHintBox, "10 minutes");
        checkBox = GuiCreateSingleCheckBox(g_autoLockHintBox, "");
        if (g_currentAutoLockIndex == 4) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoLockCheck[4] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoLockHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoLockTimeHandler, g_autoLockCheck[4]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 704);

    }
}

static void CloseChooseAutoLockTimeHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_autoLockHintBox)
    }
}

static void SelectAutoLockTimeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
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
            char *currentLockTime = GetAutoLockTimeDescByLockTime();
            lv_label_set_text(autoLockAutoLockLabel, currentLockTime);
            GUI_DEL_OBJ(g_autoLockHintBox)
        }

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

static char* GetAutoLockTimeDescByLockTime()
{
    uint32_t currentAutoLockTime = GetAutoLockScreen();

    switch (currentAutoLockTime) {
    case 15:
        g_currentAutoLockIndex = 0;
        return "15 seconds";
    case 30:
        g_currentAutoLockIndex = 1;
        return "30 seconds";
    case 60:
        g_currentAutoLockIndex = 2;
        return "1 minute";
    case 300:
        g_currentAutoLockIndex = 3;
        return "5 minutes";
    case 600:
        g_currentAutoLockIndex = 4;
        return "10 minutes";
    default:
        break;
    }
    return "";
}

static void ChooseAutoShutdownHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {

        g_autoShutdownHintBox = GuiCreateHintBox(lv_scr_act(), 480, 570, true);

        lv_obj_add_event_cb(lv_obj_get_child(g_autoShutdownHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_autoShutdownHintBox);

        lv_obj_t *label = GuiCreateIllustrateLabel(g_autoShutdownHintBox, "Auto Shutdown");
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 260);
        lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

        lv_obj_t *img = GuiCreateImg(g_autoShutdownHintBox, &imgClose);
        GuiButton_t tableHintbox = {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {0, 0} };
        lv_obj_t *buttonClose = GuiCreateButton(g_autoShutdownHintBox, 36, 36, &tableHintbox, 1, CloseChooseAutoShutdownHandler, &g_autoShutdownHintBox);
        lv_obj_align(buttonClose, LV_ALIGN_DEFAULT, 408, 257);

        label = GuiCreateTextLabel(g_autoShutdownHintBox, "1 hour");
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(g_autoShutdownHintBox, "");
        GetAutoShutdownTimeDescByLockTime();
        if (g_currentShutdownIndex == 0) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoShutdownCheck[0] = checkBox;
        GuiButton_t tables[2] = {
            {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
            {.obj = checkBox, .align = LV_ALIGN_TOP_MID, .position = {0, 24},},
        };

        lv_obj_t *button = GuiCreateButton(g_autoShutdownHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[0]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 320);


        label = GuiCreateTextLabel(g_autoShutdownHintBox, "6 hours");
        checkBox = GuiCreateSingleCheckBox(g_autoShutdownHintBox, "");
        if (g_currentShutdownIndex == 1) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoShutdownCheck[1] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoShutdownHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[1]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 416);

        label = GuiCreateTextLabel(g_autoShutdownHintBox, "12 hours");
        checkBox = GuiCreateSingleCheckBox(g_autoShutdownHintBox, "");
        if (g_currentShutdownIndex == 2) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoShutdownCheck[2] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoShutdownHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[2]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 512);

        label = GuiCreateTextLabel(g_autoShutdownHintBox, "1 day");
        checkBox = GuiCreateSingleCheckBox(g_autoShutdownHintBox, "");
        if (g_currentShutdownIndex == 3) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoShutdownCheck[3] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoShutdownHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[3]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 608);

        label = GuiCreateTextLabel(g_autoShutdownHintBox, "Never");
        checkBox = GuiCreateSingleCheckBox(g_autoShutdownHintBox, "");
        if (g_currentShutdownIndex == 4) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        g_autoShutdownCheck[4] = checkBox;
        tables[0].obj = label;
        tables[1].obj = checkBox;
        button = GuiCreateButton(g_autoShutdownHintBox, 456, 84, tables, NUMBER_OF_ARRAYS(tables), SelectAutoShutdownHandler, g_autoShutdownCheck[4]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 704);

    }
}

static void CloseChooseAutoShutdownHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_autoShutdownHintBox)
    }
}


static void SelectAutoShutdownHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
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
            char *currentPowerOffTime = GetAutoShutdownTimeDescByLockTime();
            lv_label_set_text(autoShutDownTimeLabel, currentPowerOffTime);
            GUI_DEL_OBJ(g_autoShutdownHintBox)
        }
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
static char* GetAutoShutdownTimeDescByLockTime(void)
{
    uint32_t currentPowerOff = GetAutoPowerOff();

    switch (currentPowerOff) {
    case 1:
        g_currentShutdownIndex = 0;
        return "1 hour";
    case 6:
        g_currentShutdownIndex = 1;
        return "6 hours";
    case 12:
        g_currentShutdownIndex = 2;
        return "12 hours";
    case 24:
        g_currentShutdownIndex = 3;
        return "1 day";
    case 0:
        g_currentShutdownIndex = 4;
        return "Never";
    default:
        break;
    }
    return "Never";
}