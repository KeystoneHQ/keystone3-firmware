#include "gui_button.h"
#include "gui_obj.h"

void *GuiCreateButton(lv_obj_t *parent, uint16_t w, uint16_t h, GuiButton_t *member,
                      uint8_t cnt, lv_event_cb_t buttonCb, void *param)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, w, h);
    lv_obj_set_align(cont, LV_ALIGN_DEFAULT);
    for (int i = 0; i < cnt; i++) {
        if (member[i].obj == NULL) {
            continue;
        }
        lv_obj_set_parent(member[i].obj, cont);
        lv_obj_align(member[i].obj, member[i].align, member[i].position.x, member[i].position.y);
    }
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, 0);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    if (buttonCb != NULL) {
        lv_obj_add_event_cb(cont, buttonCb, LV_EVENT_CLICKED, param);
    }
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_100, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    return cont;
}

void *GuiCreateImgButton(lv_obj_t *parent, const void *src, uint16_t width,
                         lv_event_cb_t buttonCb, void *param)
{
    GuiButton_t table[] = {
        {.obj = GuiCreateImg(parent, src), .align = LV_ALIGN_CENTER, .position = {0, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, width, width, table, NUMBER_OF_ARRAYS(table),
                                       buttonCb, param);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    return button;
}

void *GuiCreateImgLabelAdaptButton(lv_obj_t *parent, const char *text, const void *src,
                                   lv_event_cb_t buttonCb, void *param)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text);
    lv_obj_t *img = GuiCreateImg(parent, src);
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_LEFT_MID, .position = {12, 0},},
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {16 + lv_obj_get_self_width(img), 0},},
    };
    uint16_t width = lv_obj_get_self_width(label) + lv_obj_get_self_width(img) + 24;
    lv_obj_t *button = GuiCreateButton(parent, width, 36, table, NUMBER_OF_ARRAYS(table),
                                       buttonCb, param);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    return button;
}

void GuiImgLabelAdaptButtonResize(lv_obj_t *button)
{
    uint16_t width = lv_obj_get_self_width(lv_obj_get_child(button, 1)) + lv_obj_get_self_width(lv_obj_get_child(button, 0)) + 24;
    lv_obj_set_width(button, width);
}

void *GuiCreateLabelImgAdaptButton(lv_obj_t *parent, const char *text, const void *src,
                                   lv_event_cb_t buttonCb, void *param)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text);
    lv_obj_t *img = GuiCreateImg(parent, src);
    GuiButton_t table[] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {12, 0},},
        {.obj = img, .align = LV_ALIGN_LEFT_MID, .position = {16 + lv_obj_get_self_width(label), 0},},
    };
    uint16_t width = lv_obj_get_self_width(label) + lv_obj_get_self_width(img) + 24;
    lv_obj_t *button = GuiCreateButton(parent, width, 36, table, NUMBER_OF_ARRAYS(table),
                                       buttonCb, param);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    return button;
}

void *GuiCreateSelectButton(lv_obj_t *parent, const char *text, const void *src,
                            lv_event_cb_t buttonCb, void *param, bool isCling)
{
    lv_obj_t *label = GuiCreateTextLabel(parent, text);
    lv_obj_t *img = GuiCreateImg(parent, src);
    int16_t imgXpos = isCling ? 24 : 411;
    int16_t labelXpos = isCling ? 40 + lv_obj_get_self_width(img) : 24;

    GuiButton_t table[] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {labelXpos, 0},},
        {.obj = img, .align = LV_ALIGN_LEFT_MID, .position = {imgXpos, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                       buttonCb, param);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    return button;
}

void *GuiCreateStatusCoinButton(lv_obj_t *parent, const char *text, const void *src)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text);
    lv_obj_t *img = GuiCreateImg(parent, src);
    lv_img_set_pivot(img, lv_obj_get_self_width(img) / 2, 0);
    lv_img_set_zoom(img, 128);
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_TOP_MID, .position = {0, 0},},
        {.obj = label, .align = LV_ALIGN_BOTTOM_MID, .position = {0, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 296, 66, table, NUMBER_OF_ARRAYS(table),
                                       NULL, NULL);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
    return button;
}

void *GuiUpdateStatusCoinButton(lv_obj_t *button, const char *text, const void *src)
{
    lv_obj_t *label = lv_obj_get_child(button, 1);
    lv_obj_t *img = lv_obj_get_child(button, 0);
    lv_img_set_src(img, src);
    lv_img_set_pivot(img, lv_obj_get_self_width(img) / 2, 0);
    lv_img_set_zoom(img, 128);
    lv_label_set_text(label, text);
    return button;
}

void *GuiCreateSettingItemButton(lv_obj_t *parent, uint16_t width, const char *text, const char *descText, const void *src,
                                 const void *rightSrc, lv_event_cb_t buttonCb, void *param)
{
    lv_obj_t *desc = NULL, *rightImg = NULL;
    if (descText != NULL) {
        desc = GuiCreateNoticeLabel(parent, descText);
    }
    if (rightSrc != NULL) {
        rightImg = GuiCreateImg(parent, rightSrc);
    }
    GuiButton_t table[] = {
        {.obj = GuiCreateImg(parent, src), .align = LV_ALIGN_DEFAULT, .position = {24, 24},},
        {.obj = GuiCreateIllustrateLabel(parent, text), .align = LV_ALIGN_DEFAULT, .position = {76, 24},},
        {.obj = desc, .align = LV_ALIGN_DEFAULT, .position = {76, 64},},
        {.obj = rightImg, .align = LV_ALIGN_TOP_RIGHT, .position = {-24, 24},},
    };
    lv_obj_t *button = GuiCreateButton(parent, width, 84, table, NUMBER_OF_ARRAYS(table),
                                       buttonCb, param);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    return button;
}

