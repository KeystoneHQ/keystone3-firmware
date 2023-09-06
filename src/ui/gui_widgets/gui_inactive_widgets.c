/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Inactive widgets.
 * Author: leon sun
 * Create: 2023-8-4
 ************************************************************************************************/

#include "gui_inactive_widgets.h"
#include "gui_status_bar.h"
#include "gui_views.h"
#include "gui_obj.h"
#include "gui_hintbox.h"
#include "presetting.h"

static lv_obj_t *g_inactiveCont = NULL;

void GuiInactiveInit(void)
{
    lv_obj_t *tempObj;
    printf("GuiInactiveInit\n");
    if (g_inactiveCont == NULL) {
        g_inactiveCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
        tempObj = GuiCreateImg(g_inactiveCont, &imgWarn);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 180);
        tempObj = GuiCreateLittleTitleLabel(g_inactiveCont, "Inactive");
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 284);
        tempObj = GuiCreateNoticeLabel(g_inactiveCont, "The device has not been activated");
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 336);
    }
}


void GuiInactiveDeInit(void)
{

}



