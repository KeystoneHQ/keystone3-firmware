
#include <stdio.h>
#include "gui.h"
#include "gui_create_wallet_widgets.h"
#include "gui_obj.h"
#include "user_memory.h"
#include "gui_views.h"
#include "gui_hintbox.h"
#include "gui_button.h"

static lv_obj_t *g_animHintBox = NULL;
static lv_obj_t *g_imgRing = NULL;
static lv_obj_t *g_imgCircular = NULL;

void CloseHintBoxHandler(lv_event_t *e)
{
    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    void **param = lv_event_get_user_data(e);
    if (param != NULL) {
        *param = NULL;
    }
}

void GuiHintBoxResize(lv_obj_t *obj, uint16_t height)
{
    lv_obj_t *upCont = lv_obj_get_child(obj, 0);
    lv_obj_set_height(upCont, 800 - height);
    lv_obj_align(upCont, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *midCont = lv_obj_get_child(obj, 1);
    lv_obj_align(midCont, LV_ALIGN_TOP_MID, 0, 800 - height);

    lv_obj_t *downCont = lv_obj_get_child(obj, 2);
    lv_obj_set_height(downCont, height - 80 + 20);
    lv_obj_align(downCont, LV_ALIGN_BOTTOM_MID, 0, 0);
}

void *GuiCreateHintBox(uint16_t h)
{
    lv_obj_t *bgCont = GuiCreateContainer(480, 800);
    lv_obj_set_style_bg_opa(bgCont, 0, 0);
    lv_obj_align(bgCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(bgCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *upCont = GuiCreateContainerWithParent(bgCont, 480, 800 - h);
    lv_obj_set_style_bg_opa(upCont, 0, 0);
    lv_obj_align(upCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(upCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(upCont, LV_OPA_30, 0);

    lv_obj_t *midCont = GuiCreateContainerWithParent(bgCont, 480, 80);
    lv_obj_set_style_bg_color(midCont, DARK_BG_COLOR,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(midCont, 20, 0);
    lv_obj_align(midCont, LV_ALIGN_TOP_MID, 0, 800 - h);
    lv_obj_add_flag(midCont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *downCont = GuiCreateContainerWithParent(bgCont, 480, h - 80 + 20);
    lv_obj_set_style_bg_color(downCont, DARK_BG_COLOR,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(downCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(downCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(downCont, LV_OBJ_FLAG_CLICKABLE);

    return bgCont;
}

void *GuiCreateHintBoxWithoutTop(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *bgCont = GuiCreateContainerWithParent(parent, w, h);
    lv_obj_set_style_bg_color(bgCont, DARK_BG_COLOR,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
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

void GuiStopAnimHintBox(void)
{
    if (g_animHintBox != NULL) {
        lv_anim_del_all();
    }

    if (g_imgRing != NULL) {
        lv_obj_add_flag(g_imgRing, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_imgCircular != NULL) {
        lv_obj_add_flag(g_imgCircular, LV_OBJ_FLAG_HIDDEN);
    }
}

void *GuiCreateAnimHintBox(uint16_t w, uint16_t h, uint16_t animHeight)
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
    lv_obj_set_style_bg_color(midCont, DARK_BG_COLOR,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(midCont, 20, 0);
    lv_obj_align(midCont, LV_ALIGN_TOP_MID, 0, 800 - h);

    lv_obj_t *downCont = GuiCreateContainerWithParent(bgCont, w, h - 80 + 20);
    lv_obj_set_style_bg_color(downCont, DARK_BG_COLOR,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(downCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(downCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(downCont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *img = GuiCreateImg(downCont, &imgRing);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, animHeight - 60);
    g_imgRing = img;

    img = GuiCreateImg(downCont, &imgCircular);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, animHeight - 55);
    lv_img_set_pivot(img, 5, 25);
    g_imgCircular = img;

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

void *GuiCreateResultHintbox(uint16_t h, const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText,
                             lv_color_t leftColor, const char *rightBtnText,
                             lv_color_t rightColor)
{
    lv_obj_t *cont = GuiCreateHintBox(h);
    lv_obj_t *desc = GuiCreateNoticeLabel(cont, descText);
    lv_obj_align(desc, LV_ALIGN_BOTTOM_LEFT, 36, -130);
    lv_obj_t *title = GuiCreateLittleTitleLabel(cont, titleText);
    lv_obj_align_to(title, desc, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
    lv_obj_t *img = GuiCreateImg(cont, src);
    lv_obj_align_to(img, title, LV_ALIGN_OUT_TOP_LEFT, 0, -24);

    if (leftBtnText != NULL) {
        lv_obj_t *leftBtn = GuiCreateTextBtn(cont, leftBtnText);
        lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_set_size(
            leftBtn, lv_obj_get_self_width(lv_obj_get_child(leftBtn, 0)) + 60, 66);
        lv_obj_set_style_bg_color(leftBtn, leftColor, LV_PART_MAIN);
    }

    lv_obj_t *rightBtn = GuiCreateTextBtn(cont, rightBtnText);
    lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(
        rightBtn, lv_obj_get_self_width(lv_obj_get_child(rightBtn, 0)) + 60, 66);
    lv_obj_set_style_bg_color(rightBtn, rightColor, LV_PART_MAIN);
    return cont;
}

void *GuiCreateGeneralHintBox(const void *src, const char *titleText,
                              const char *desc1Text, const char *desc2Text,
                              const char *leftBtnText, lv_color_t leftColor,
                              const char *rightBtnText, lv_color_t rightColor)
{
    lv_obj_t *img = NULL, *title = NULL, *desc1 = NULL, *desc2 = NULL,
              *leftBtn = NULL, *rightBtn = NULL;
    lv_obj_t *cont = GuiCreateHintBox(800);
    if (desc2Text != NULL) {
        desc2 = GuiCreateIllustrateLabel(cont, desc2Text);
        lv_obj_align(desc2, LV_ALIGN_BOTTOM_LEFT, 36, -130);
    }
    if (desc1Text != NULL) {
        desc1 = GuiCreateIllustrateLabel(cont, desc1Text);
        if (desc2 == NULL) {
            lv_obj_align(desc1, LV_ALIGN_BOTTOM_LEFT, 36, -130);
        } else {
            lv_obj_align_to(desc1, desc2, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
        }
    }
    title = GuiCreateLittleTitleLabel(cont, titleText);
    if (desc1 == NULL) {
        lv_obj_align(title, LV_ALIGN_BOTTOM_LEFT, 36, -130);
    } else {
        lv_obj_align_to(title, desc1, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
    }
    if (src != NULL) {
        img = GuiCreateImg(cont, src);
        lv_obj_align_to(img, title, LV_ALIGN_OUT_TOP_LEFT, 0, -24);
    }

    if (leftBtnText != NULL) {
        leftBtn = GuiCreateTextBtn(cont, leftBtnText);
        lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 36, -20);
        lv_obj_set_size(leftBtn, lv_obj_get_self_width(lv_obj_get_child(leftBtn, 0)) + 40, 66);
        lv_obj_set_style_bg_color(leftBtn, leftColor, LV_PART_MAIN);
    }

    if (rightBtnText != NULL) {
        rightBtn = GuiCreateTextBtn(cont, rightBtnText);
        lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -20);
        lv_obj_set_size(rightBtn, lv_obj_get_self_width(lv_obj_get_child(rightBtn, 0)) + 40, 66);
        lv_obj_set_style_bg_color(rightBtn, rightColor, LV_PART_MAIN);
    }

    uint32_t height =
        24 + lv_obj_get_self_height(title) + 12 + lv_obj_get_self_height(desc1);
    if (img != NULL) {
        height = height + 48 + lv_obj_get_self_width(img);
    }
    if (lv_obj_get_self_height(desc2) != 0) {
        height += (12 + lv_obj_get_self_height(desc2));
    }
    height += 16;
    height += 114;

    GuiHintBoxResize(cont, height);

    return cont;
}

void *GuiCreateUpdateHintbox(const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText,
                             lv_color_t leftColor, const char *rightBtnText,
                             lv_color_t rightColor, bool checkSumDone)
{
    lv_obj_t *cont = GuiCreateHintBox(800);
    lv_obj_t *checksum =
        GuiCreateIllustrateLabel(cont, _("firmware_update_sd_checksum_desc"));
    if (checkSumDone) {
        lv_label_set_text(checksum, _("firmware_update_sd_checksum_done"));
    }
    lv_label_set_recolor(checksum, true);
    lv_obj_set_style_text_font(checksum, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_align(checksum, LV_ALIGN_BOTTOM_LEFT, 36, -130);
    lv_obj_t *desc = GuiCreateNoticeLabel(cont, descText);
    if (checkSumDone) {
        lv_obj_align(desc, LV_ALIGN_BOTTOM_LEFT, 36, -232);
    } else {
        lv_obj_align_to(desc, checksum, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
    }
    lv_obj_t *title = GuiCreateLittleTitleLabel(cont, titleText);
    lv_obj_align_to(title, desc, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
    lv_obj_t *img = GuiCreateImg(cont, src);
    lv_obj_align_to(img, title, LV_ALIGN_OUT_TOP_LEFT, 0, -24);

    if (leftBtnText != NULL) {
        lv_obj_t *leftBtn = GuiCreateTextBtn(cont, leftBtnText);
        lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_set_size(
            leftBtn, lv_obj_get_self_width(lv_obj_get_child(leftBtn, 0)) + 60, 66);
        lv_obj_set_style_bg_color(leftBtn, leftColor, LV_PART_MAIN);
    }

    lv_obj_t *rightBtn = GuiCreateTextBtn(cont, rightBtnText);
    lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(rightBtn, lv_obj_get_self_width(lv_obj_get_child(rightBtn, 0)) + 60, 66);
    lv_obj_set_style_bg_color(rightBtn, rightColor, LV_PART_MAIN);
    uint32_t height = 48 + lv_obj_get_self_width(img) + 24 +
                      lv_obj_get_self_height(title) + 12 +
                      lv_obj_get_self_height(desc) + 12 +
                      lv_obj_get_self_height(checksum) + 16 + 24 + 66 + 24;
    GuiHintBoxResize(cont, height);

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

void *GuiCreateMoreInfoHintBox(const void *src, const char *titleText,
                               MoreInfoTable_t *table, uint8_t cnt,
                               bool isCling, void *parent)
{
    lv_obj_t *title = NULL;
    lv_obj_t *cont = GuiCreateHintBox(800);
    uint32_t height = 12 + 12;

    for (int8_t i = cnt - 1; i >= 0; --i) {
        lv_obj_t *btn =
            GuiCreateSelectButton(cont, table[i].name, table[i].src,
                                  table[i].callBack, table[i].param, isCling);
        if (i == cnt - 1) {
            lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -12);
        } else {
            GuiAlignToPrevObj(btn, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
        }
        height += 96;
    }

    if (titleText != NULL) {
        title = GuiCreateIllustrateLabel(cont, titleText);
        GuiAlignToPrevObj(title, LV_ALIGN_OUT_TOP_LEFT, 24, -24);
        height += 66;
    }

    if (src != NULL) {
        lv_obj_t *btn =
            GuiCreateImgButton(cont, src, 64, CloseHintBoxHandler, parent);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -24, 64 + 24 - height);
    }
    lv_obj_add_event_cb(lv_obj_get_child(cont, 0), CloseHintBoxHandler,
                        LV_EVENT_CLICKED, parent);
    GuiHintBoxResize(cont, height);

    return cont;
}