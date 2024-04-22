#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_status_bar.h"
#include "gui_views.h"
#include "gui_hintbox.h"
#include "gui_setup_widgets.h"
#include "presetting.h"
#include "version.h"

static lv_obj_t *g_updateSuccessCont = NULL;

static void UpdateSuccessNextStepHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_updateSuccessCont)
    GuiCLoseCurrentWorkingView();

        if (ModelGetPassphraseQuickAccess()) {
            GuiFrameOpenView(&g_passphraseView);
        } else {
            GuiFrameOpenView(&g_homeView);
        }
    }
}

void GuiUpdateSuccessInit(void)
{
    lv_obj_t *tempObj;
    char version[32] = "v";
    if (g_updateSuccessCont == NULL) {
        g_updateSuccessCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
        lv_obj_add_flag(g_updateSuccessCont, LV_OBJ_FLAG_CLICKABLE);
        tempObj = GuiCreateImg(g_updateSuccessCont, &imgUpdate);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 180);
        tempObj = GuiCreateLittleTitleLabel(g_updateSuccessCont, "Update Successful");
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 284);
        GetSoftWareVersionNumber(&version[1]);
        tempObj = GuiCreateNoticeLabel(g_updateSuccessCont, version);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 336);

        lv_obj_t *btn = GuiCreateBtn(g_updateSuccessCont, USR_SYMBOL_ARROW_NEXT);
        lv_obj_set_size(btn, 96, 96);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -96);
        lv_obj_add_event_cb(btn, UpdateSuccessNextStepHandler, LV_EVENT_CLICKED, NULL);
    }
}


