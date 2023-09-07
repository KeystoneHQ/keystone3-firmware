
#include <stdio.h>
#include "gui.h"
#include "gui_create_wallet_widgets.h"
#include "gui_obj.h"
#include "user_memory.h"
#include "gui_views.h"

static lv_obj_t *g_animHintBox = NULL;

void CloseHintBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        void **param = lv_event_get_user_data(e);
        if (param != NULL) {
            *param = NULL;
        }
    }
}

void *GuiCreateHintBox(lv_obj_t *parent, uint16_t w, uint16_t h, bool en)
{
    lv_obj_t *bgCont = GuiCreateContainer(w, 800);
    lv_obj_set_style_bg_opa(bgCont, 0, 0);
    lv_obj_align(bgCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *upCont = GuiCreateContainerWithParent(bgCont, w, 800 - h);
    lv_obj_set_style_bg_opa(upCont, 0, 0);
    lv_obj_align(upCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(upCont, LV_OBJ_FLAG_CLICKABLE);
    // if (en) {
    //     lv_obj_add_event_cb(upCont, CloseHintBoxHandler, LV_EVENT_CLICKED, bgCont);
    // }
    lv_obj_set_style_bg_opa(upCont, LV_OPA_30, 0);

    lv_obj_t *midCont = GuiCreateContainerWithParent(bgCont, w, 80);
    lv_obj_set_style_bg_color(midCont, DARK_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(midCont, 20, 0);
    lv_obj_align(midCont, LV_ALIGN_TOP_MID, 0, 800 - h);
    lv_obj_add_flag(midCont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *downCont = GuiCreateContainerWithParent(bgCont, w, h - 80 + 20);
    lv_obj_set_style_bg_color(downCont, DARK_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(downCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(downCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(downCont, LV_OBJ_FLAG_CLICKABLE);

    return bgCont;
}

void *GuiCreateHintBoxWithoutTop(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *bgCont = GuiCreateContainerWithParent(parent, w, h);
    //    lv_obj_set_style_bg_opa(bgCont, 0, 0);
    lv_obj_set_style_bg_color(bgCont, DARK_BG_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(bgCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return bgCont;
}

void GuiDeleteAnimHintBox(void)
{
    if (g_animHintBox != NULL) {
        lv_obj_del(g_animHintBox);
        g_animHintBox = NULL;
        lv_anim_del_all();
    }
}

void *GuiCreateAnimHintBox(lv_obj_t *parent, uint16_t w, uint16_t h, uint16_t animHeight)
{
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
    g_animHintBox = bgCont;
    return bgCont;
}

uint16_t GetHintBoxReHeight(uint16_t oldHeight, lv_obj_t *obj)
{
    return (oldHeight += lv_obj_get_self_height(obj));
}

void *GuiCreateResultHintbox(lv_obj_t *parent, uint16_t h, const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText, lv_color_t leftColor, const char *rightBtnText, lv_color_t rightColor)
{
    lv_obj_t *cont = GuiCreateHintBox(lv_scr_act(), 480, h, false);
    lv_obj_t *desc = GuiCreateNoticeLabel(cont, descText);
    lv_obj_align(desc, LV_ALIGN_BOTTOM_LEFT, 36, -130);
    lv_obj_t *title = GuiCreateLittleTitleLabel(cont, titleText);
    lv_obj_align_to(title, desc, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
    lv_obj_t *img = GuiCreateImg(cont, src);
    lv_obj_align_to(img, title, LV_ALIGN_OUT_TOP_LEFT, 0, -24);

    if (leftBtnText != NULL) {
        lv_obj_t *leftBtn = GuiCreateBtn(cont, leftBtnText);
        lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_set_size(leftBtn, lv_obj_get_self_width(lv_obj_get_child(leftBtn, 0)) + 60, 66);
        lv_obj_set_style_bg_color(leftBtn, leftColor, LV_PART_MAIN);
    }

    lv_obj_t *rightBtn = GuiCreateBtn(cont, rightBtnText);
    lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(rightBtn, lv_obj_get_self_width(lv_obj_get_child(rightBtn, 0)) + 60, 66);
    lv_obj_set_style_bg_color(rightBtn, rightColor, LV_PART_MAIN);
    // lv_obj_set_size(rightBtn, 122, 66);
    // lv_obj_add_event_cb(btn, DelCurrCloseToSubtopViewHandler, LV_EVENT_CLICKED, NULL);
    return cont;
}

void *GuiGetHintBoxRightBtn(lv_obj_t *parent)
{
    return lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) - 1);
}

void *GuiGetHintBoxLeftBtn(lv_obj_t *parent)
{
    return lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) - 2);
}
