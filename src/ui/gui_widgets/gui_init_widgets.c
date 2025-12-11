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
