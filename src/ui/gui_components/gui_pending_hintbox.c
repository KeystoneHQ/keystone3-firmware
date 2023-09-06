#include "gui.h"
#include "lvgl.h"
#include "gui_obj.h"

static lv_obj_t *g_pendingHintBox = NULL;

void GuiPendingHintBoxOpen(char* title, char* subtitle)
{
    uint16_t h = 326;
    uint16_t animHeight = 82;
    uint16_t w = 480;
    lv_obj_t *bgCont = GuiCreateContainer(w, 800);
    lv_obj_set_style_bg_opa(bgCont, 0, 0);
    lv_obj_align(bgCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *upCont = GuiCreateContainerWithParent(bgCont, w, 800 - h);
    lv_obj_set_style_bg_opa(upCont, 0, 0);
    lv_obj_add_flag(upCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(upCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(upCont, LV_OPA_30, 0);

    lv_obj_t *midCont = GuiCreateContainerWithParent(bgCont, w, 80);
    lv_obj_set_style_bg_color(midCont, DARK_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(midCont, 20, 0);
    lv_obj_align(midCont, LV_ALIGN_TOP_MID, 0, 800 - h);

    lv_obj_t *downCont = GuiCreateContainerWithParent(bgCont, w, h - 80 + 20);
    lv_obj_set_style_bg_color(downCont, DARK_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(downCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(downCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *img = GuiCreateImg(downCont, &ring);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, animHeight - 60);

    img = GuiCreateImg(downCont, &circular);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, animHeight - 55);
    lv_img_set_pivot(img, 5, 25);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, GuiSetAngle);
    lv_anim_set_values(&a, 0, 3600);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_obj_t *label = GuiCreateTextLabel(bgCont, title);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -124);
    label = GuiCreateNoticeLabel(bgCont, subtitle);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -76);

    g_pendingHintBox = bgCont;
}

void GuiPendingHintBoxRemove()
{
    if (g_pendingHintBox != NULL) {
        GUI_DEL_OBJ(g_pendingHintBox)
        lv_anim_del_all();
    }
}