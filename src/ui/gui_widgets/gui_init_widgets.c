#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_about_info_widgets.h"
#include "gui_page.h"
#ifndef COMPILE_SIMULATOR
#include "drv_battery.h"
#else
#include "simulator_model.h"
#endif

static lv_obj_t *g_bootNotMatchCont = NULL;

void GuiBootVersionNotMatchWidget(void)
{
    g_bootNotMatchCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_add_flag(g_bootNotMatchCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *label = GuiCreateTextLabel(g_bootNotMatchCont, _("boot_version_not_match_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 180);
    label = GuiCreateNoticeLabel(g_bootNotMatchCont, _("boot_version_not_match_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 284);
}
