#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_about_info_widgets.h"
#include "gui_page.h"
#include "presetting.h"
#include "version.h"
#ifndef COMPILE_SIMULATOR
#include "drv_battery.h"
#else
#include "simulator_model.h"
#endif

static lv_obj_t *g_bootNotMatchCont = NULL;
static lv_timer_t *g_countDownTimer = NULL;                 // count down timer

static void CountDownTimerHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    static int8_t countDown = 30;
    char buf[32] = {0};
    --countDown;
    if (countDown > 0) {
        snprintf_s(buf, sizeof(buf), "%s(%d)", _("enter_system"), countDown);
    } else {
        strcpy_s(buf, sizeof(buf), _("enter_system"));
    }
    lv_label_set_text(lv_obj_get_child(obj, 0), buf);
    if (countDown <= 0) {
        lv_obj_set_style_bg_opa(obj, LV_OPA_100, LV_PART_MAIN);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_timer_del(timer);
        countDown = 30;
        g_countDownTimer = NULL;
        UNUSED(g_countDownTimer);
    }
}

static void GuiEnterSystemHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_bootNotMatchCont)
    GuiModeGetAccount();
}

void GuiBootVersionNotMatchWidget(void)
{
    g_bootNotMatchCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_add_flag(g_bootNotMatchCont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *img = GuiCreateImg(g_bootNotMatchCont, &imgInformation);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 182);

    lv_obj_t *label = GuiCreateTextLabel(g_bootNotMatchCont, _("boot_version_not_match_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 286);
    label = GuiCreateIllustrateLabel(g_bootNotMatchCont, _("boot_version_not_match_desc"));
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 338);

    char buffer[BUFFER_SIZE_64];
    char tempBuffer[BUFFER_SIZE_64];
    GetSoftWareVersionNumber(buffer);
    snprintf(tempBuffer, BUFFER_SIZE_64, "Firmware version: %s", buffer);
    label = GuiCreateNoticeLabel(g_bootNotMatchCont, tempBuffer);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 538);

    GetBootVersionNumber(buffer);
    memset(tempBuffer, 0, sizeof(tempBuffer));
    snprintf(tempBuffer, BUFFER_SIZE_64, "Boot version: %s", buffer);
    label = GuiCreateNoticeLabel(g_bootNotMatchCont, tempBuffer);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 588);

    GetSerialNumber(buffer);
    memset(tempBuffer, 0, sizeof(tempBuffer));
    snprintf(tempBuffer, BUFFER_SIZE_64, "Serial number: %s", buffer);
    label = GuiCreateNoticeLabel(g_bootNotMatchCont, tempBuffer);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 638);

    lv_obj_t *btn = GuiCreateTextBtn(g_bootNotMatchCont, _("enter_system"));
    lv_obj_add_event_cb(btn, GuiEnterSystemHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(btn, LV_OPA_60, LV_STATE_DEFAULT);
    g_countDownTimer = lv_timer_create(CountDownTimerHandler, 1000, btn);
}
