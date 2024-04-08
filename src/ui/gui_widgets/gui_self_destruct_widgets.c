#include "gui_self_destruct_widgets.h"
#include "gui_status_bar.h"
#include "gui_views.h"
#include "gui_obj.h"
#include "gui_hintbox.h"
#include "presetting.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

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
        tempObj = GuiCreateLittleTitleLabel(g_selfDestructCont, _("self_destruction_title"));
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -476);
        tempObj = GuiCreateNoticeLabel(g_selfDestructCont, _("self_destruction_desc"));
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(tempObj, 408);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -344);
        tempObj = GuiCreateNoticeLabel(g_selfDestructCont, _("self_destruction_hint"));
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -148);
        tempObj = GuiCreateIllustrateLabel(g_selfDestructCont, _("support_link"));
        lv_obj_set_style_text_color(tempObj, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -106);
        strcpy_s(serialNum, 3, "SN:");
        GetSerialNumber(serialNum + strnlen_s(serialNum, 3));
        tempObj = GuiCreateNoticeLabel(g_selfDestructCont, serialNum);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -36);
    }
}

void GuiSelfDestructDeInit(void)
{

}
