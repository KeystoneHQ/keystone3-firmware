/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Self destruct widgets.
 * Author: leon sun
 * Create: 2023-8-2
 ************************************************************************************************/


#include "gui_self_destruct_widgets.h"
#include "gui_status_bar.h"
#include "gui_views.h"
#include "gui_obj.h"
#include "gui_hintbox.h"
#include "presetting.h"

static lv_obj_t *g_selfDestructCont = NULL;

void GuiSelfDestructInit(void)
{
    lv_obj_t *tempObj;
    char serialNum[64];
    printf("GuiSelfDestructInit\n");
    if (g_selfDestructCont == NULL) {
        g_selfDestructCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
        lv_obj_align(g_selfDestructCont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);
        tempObj = GuiCreateImg(g_selfDestructCont, &imgLockDestroy);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -548);
        tempObj = GuiCreateLittleTitleLabel(g_selfDestructCont, "Device No Longer Usable");
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -476);
        tempObj = GuiCreateNoticeLabel(g_selfDestructCont, "Physical attack detected, all sensitive information on this device has been completely erased and this device can no longer be usable.");
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(tempObj, 408);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -344);
        tempObj = GuiCreateNoticeLabel(g_selfDestructCont, "Contact us if any questions:");
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -148);
        tempObj = GuiCreateIllustrateLabel(g_selfDestructCont, "support@keyst.one");
        lv_obj_set_style_text_color(tempObj, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -106);
        strcpy(serialNum, "SN:");
        GetSerialNumber(serialNum + strlen(serialNum));
        tempObj = GuiCreateNoticeLabel(g_selfDestructCont, serialNum);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -36);
    }
}


void GuiSelfDestructDeInit(void)
{

}

