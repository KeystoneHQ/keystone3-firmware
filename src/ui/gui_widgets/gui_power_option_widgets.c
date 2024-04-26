#include "gui_power_option_widgets.h"
#include "lvgl.h"
#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_api.h"

static void GuiPowerOptionInit(void);
static void GuiPowerOptionDeInit(void);
static void PowerOffHandler(lv_event_t *e);
void RebootHandler(lv_event_t *e);
static void CancelHandler(lv_event_t *e);

static lv_obj_t *container;

const GuiMsgBox_t g_guiMsgBoxPowerOption = {
    GuiPowerOptionInit,
    GuiPowerOptionDeInit,
    GUI_POWER_OPTION_PRIORITY,
};

static void GuiPowerOptionInit(void)
{
    lv_obj_t *btn, *img, *label;
    if (container != NULL) {
        lv_obj_del(container);
    }
    container = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    btn = GuiCreateBtn(container, "");
    lv_obj_set_size(btn, 96, 96);
    lv_obj_set_style_radius(btn, 48, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(btn, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_10, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 184);
    img = GuiCreateImg(btn, &imgPowerOff);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(btn, PowerOffHandler, LV_EVENT_CLICKED, NULL);

    btn = GuiCreateBtn(container, "");
    lv_obj_set_size(btn, 96, 96);
    lv_obj_set_style_radius(btn, 48, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(btn, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_10, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 374);
    img = GuiCreateImg(btn, &imgReboot);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(btn, RebootHandler, LV_EVENT_CLICKED, NULL);

    label = GuiCreateNoticeLabel(container, _("power_off"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 296);
    label = GuiCreateNoticeLabel(container, _("Restart"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 486);

    btn = GuiCreateBtn(container, _("Cancel"));
    label = lv_obj_get_child(btn, 0);
    lv_obj_set_style_text_font(label, g_defTextFont, LV_PART_MAIN);
    lv_obj_set_size(btn, 135, 66);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -64);
    lv_obj_add_event_cb(btn, CancelHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiPowerOptionDeInit(void)
{
    if (container != NULL) {
        lv_obj_del(container);
    }
    container = NULL;
}

#ifndef COMPILE_SIMULATOR
#include "background_task.h"
#endif

static void PowerOffHandler(lv_event_t *e)
{
    printf("power off\r\n");
#ifndef COMPILE_SIMULATOR
    SystemPoweroff();
#endif
}

void RebootHandler(lv_event_t *e)
{
    printf("reboot\r\n");
#ifndef COMPILE_SIMULATOR
    SystemReboot();
#endif
}

static void CancelHandler(lv_event_t *e)
{
    printf("cancel\r\n");
    GuiApiEmitSignalWithValue(SIG_INIT_POWER_OPTION, 0);
}
