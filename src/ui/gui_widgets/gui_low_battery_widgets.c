#include "gui_low_battery_widgets.h"
#include "gui.h"
#include "gui_pop_message_box.h"
#include "gui_hintbox.h"
#include "gui_views.h"
#include "gui_api.h"

static void GuiLowBatteryInit(void);
static void GuiLowBatteryDeInit(void);
static void ButtonHandler(lv_event_t *e);

static lv_obj_t *g_lowBatteryHintBox = NULL;

const GuiMsgBox_t g_guiMsgBoxLowBattery = {
    GuiLowBatteryInit,
    GuiLowBatteryDeInit,
    GUI_LOW_BATTERY_PRIORITY,
};

static void GuiLowBatteryInit(void)
{
    lv_obj_t *img, *label, *button;
    if (g_lowBatteryHintBox == NULL) {
        g_lowBatteryHintBox = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
        img = GuiCreateImg(g_lowBatteryHintBox, &imgWarn);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 36, -296);

        label = GuiCreateLittleTitleLabel(g_lowBatteryHintBox, _("low_battery_pop_up_title"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -232);
        label = GuiCreateIllustrateLabel(g_lowBatteryHintBox, _("low_battery_pop_up_desc"));
        lv_obj_set_width(label, 408);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -130);

        button = GuiCreateBtnWithFont(g_lowBatteryHintBox, _("OK"), g_defTextFont);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_set_size(button, 94, 66);
        lv_obj_set_style_bg_color(button, DARK_GRAY_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(button, ButtonHandler, LV_EVENT_CLICKED, NULL);
    }
}

static void GuiLowBatteryDeInit(void)
{
    if (g_lowBatteryHintBox) {
        lv_obj_del(g_lowBatteryHintBox);
        g_lowBatteryHintBox = NULL;
    }
}

static void ButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiApiEmitSignalWithValue(SIG_INIT_LOW_BATTERY, 0);
    }
}
