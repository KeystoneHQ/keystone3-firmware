#include "gui_obj.h"

lv_obj_t *g_circleAnimationItems = NULL;
lv_anim_t g_circleAnimation = {0};
lv_obj_t *g_ringImg = NULL;
lv_obj_t *g_dotImg = NULL;

void *GuiCreateContainerWithParent(lv_obj_t *parent, int w, int h)
{
    lv_obj_t *cont = lv_obj_create(parent);
    if (cont == NULL) {
        return NULL;
    }
    lv_obj_set_size(cont, w, h);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(cont, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    }
    return cont;
}

void *GuiCreateLabelWithFontScroll(lv_obj_t *parent, const char *text,
                                   const lv_font_t *font, uint16_t width)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    if (GuiDarkMode()) {
        lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(label, BLACK_COLOR, LV_PART_MAIN);
    }
    if (lv_obj_get_self_width(label) >= width) {
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_width(label, width);
    }
    return label;
}

void *GuiCreateLabelWithFont(lv_obj_t *parent, const char *text,
                             const lv_font_t *font)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    if (GuiDarkMode()) {
        lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(label, BLACK_COLOR, LV_PART_MAIN);
    }
    if (lv_obj_get_self_width(label) >= 400) {
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(label, 408);
    }
    return label;
}

void *GuiCreateWhiteOpa12Container(lv_obj_t *parent, int w, int h)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, w, h);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_12, LV_PART_MAIN);

    return cont;
}

void *GuiCreateLabelWithFontAndTextColor(lv_obj_t *parent, const char *text,
    const lv_font_t *font, int color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    if (GuiDarkMode()) {
        lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF - color),
                                    LV_PART_MAIN);
    }
    if (lv_obj_get_self_width(label) >= 400) {
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(label, 408);
    }
    return label;
}

void *GuiCreateNoticeLabel(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = GuiCreateLabelWithFont(parent, text, g_defIllustrateFont);
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    return label;
}

void *GuiCreateImg(lv_obj_t *parent, const void *src)
{
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, src);
    return img;
}

void *GuiCreateScaleImg(lv_obj_t *parent, const void *src, int scale)
{
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, src);
    lv_img_set_pivot(img, 0, 0);
    lv_img_set_zoom(img, scale);
    return img;
}

void *GuiCreateBtnWithFont(lv_obj_t *parent, const char *text,
                           const lv_font_t *font)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_t *label = GuiCreateTextLabel(btn, text);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_size(btn, lv_obj_get_self_width(label) + 24,
                    lv_obj_get_self_height(label) + 6);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    return btn;
}

void *GuiCreateTextBtn(lv_obj_t *parent, const char *text)
{
    lv_obj_t *btn = GuiCreateBtnWithFont(parent, text, g_defTextFont);
    uint16_t width = lv_obj_get_self_width(lv_obj_get_child(btn, 0)) + 24;
    width = width > 96 ? width : 96;
    lv_obj_set_size(btn, width, 66);
    return btn;
}

void *GuiCreateBtn(lv_obj_t *parent, const char *text)
{
    lv_obj_t *btn = GuiCreateBtnWithFont(parent, text, &buttonFont);
    lv_obj_set_size(btn, 96, 66);
    return btn;
}

void *GuiCreateAdaptButton(lv_obj_t *parent, const char *text)
{
    lv_obj_t *btn = GuiCreateBtnWithFont(parent, text, &buttonFont);
    uint16_t width = lv_obj_get_self_width(lv_obj_get_child(btn, 0)) + 24;
    width = width > 96 ? width : 96;
    lv_obj_set_size(btn, width, 66);
    return btn;
}

void *GuiCreateCheckBoxWithFont(lv_obj_t *parent, const char *text, bool single,
                                const lv_font_t *font)
{
    lv_obj_t *checkBox = lv_checkbox_create(parent);
    lv_obj_add_flag(checkBox, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_size(checkBox, 420, 190);
    lv_checkbox_set_text(checkBox, text);
    lv_obj_set_style_text_color(checkBox, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(checkBox, font, LV_PART_MAIN);
    lv_obj_set_style_radius(checkBox, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_border_opa(checkBox, LV_OPA_30, LV_PART_INDICATOR);
    lv_obj_set_style_border_color(checkBox, WHITE_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(checkBox, 2, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(checkBox, DARK_BG_COLOR,
                              LV_PART_MAIN | LV_PART_INDICATOR);

    // check
    if (single) {
        lv_obj_set_style_bg_color(checkBox, ORANGE_COLOR,
                                  LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_bg_img_src(checkBox, &imgMessageRight,
                                    LV_PART_INDICATOR | LV_STATE_CHECKED);
    } else {
        lv_obj_set_style_bg_color(checkBox, ORANGE_COLOR,
                                  LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_bg_img_src(checkBox, &imgMessageRight,
                                    LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
    lv_obj_set_style_border_opa(checkBox, LV_OPA_0,
                                LV_PART_INDICATOR | LV_STATE_CHECKED);
    return checkBox;
}

void *GuiCreateSelectPathCheckBox(lv_obj_t *parent)
{
    lv_obj_t *checkBox = lv_btn_create(parent);
    lv_obj_set_size(checkBox, 408, 82);
    lv_obj_set_style_bg_opa(checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
    lv_obj_set_style_border_width(checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(checkBox, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(checkBox, 0, LV_PART_MAIN);
    lv_obj_add_flag(checkBox, LV_OBJ_FLAG_CHECKABLE);

    return checkBox;
}

void *GuiCreateLine(lv_obj_t *parent, lv_point_t linePoints[],
                    uint16_t pointNum)
{
    lv_obj_t *line = lv_line_create(parent);
    lv_line_set_points(line, linePoints, pointNum);
    lv_obj_add_style(line, &g_dividerLineStyle, 0);
    return line;
}

void *GuiCreateDividerLine(lv_obj_t *parent)
{
    static lv_point_t linePoints[2] = {{36, 0}, {444, 0}};
    return GuiCreateLine(parent, linePoints, 2);
}

void GuiSetAngle(void *img, int32_t v)
{
    lv_img_set_angle(img, v);
}

void *GuiCreateCircleAroundAnimation(lv_obj_t *parent, int w)
{
    GuiStopCircleAroundAnimation();
    lv_obj_set_size(parent, 480, 800);
    g_ringImg = GuiCreateImg(parent, &imgRing);
    lv_obj_align(g_ringImg, LV_ALIGN_CENTER, 0, w);

    g_dotImg = GuiCreateImg(parent, &imgCircular);
    lv_obj_align(g_dotImg, LV_ALIGN_CENTER, 0, w - 20);
    lv_img_set_pivot(g_dotImg, 5, 25);

    lv_anim_init(&g_circleAnimation);
    lv_anim_set_var(&g_circleAnimation, g_dotImg);
    lv_anim_set_exec_cb(&g_circleAnimation, GuiSetAngle);
    lv_anim_set_values(&g_circleAnimation, 0, 3600);
    lv_anim_set_time(&g_circleAnimation, 1000);
    lv_anim_set_repeat_count(&g_circleAnimation, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&g_circleAnimation);
    return NULL;
}

void GuiStopCircleAroundAnimation(void)
{
    lv_anim_del_all();
    if (g_ringImg != NULL && lv_obj_is_valid(g_ringImg)) {
        lv_obj_del(g_ringImg);
        g_ringImg = NULL;
    }
    if (g_dotImg != NULL && lv_obj_is_valid(g_dotImg)) {
        lv_obj_del(g_dotImg);
        g_dotImg = NULL;
    }
}

void *GuiCreateConfirmSlider(lv_obj_t *parent, lv_event_cb_t cb)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 480, 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);

    lv_obj_t *bgCont = GuiCreateContainerWithParent(cont, 408, 74);
    lv_obj_set_style_bg_color(bgCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bgCont, LV_OPA_10, LV_PART_MAIN);
    lv_obj_set_align(bgCont, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(bgCont, 24, LV_PART_MAIN);
    lv_obj_t *label = GuiCreateIllustrateLabel(
                          bgCont, _("scan_qr_code_sign_unsigned_content_frame"));
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_align(label, LV_ALIGN_CENTER);

    lv_obj_t *slider = lv_slider_create(bgCont);
    lv_obj_add_event_cb(slider, cb, LV_EVENT_RELEASED, NULL);
    lv_obj_remove_style_all(slider);
    lv_obj_set_size(slider, 305, 74);
    lv_obj_set_align(slider, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(slider, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_radius(slider, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(slider, 0, LV_PART_MAIN);

    lv_obj_set_style_bg_img_src(slider, &imgConfirmSlider, LV_PART_KNOB);
    lv_obj_set_style_border_width(slider, 0, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 20, LV_PART_KNOB);
    return slider;
}

void *GuiCreateTileView(lv_obj_t *parent)
{
    lv_obj_t *tileView = lv_tileview_create(parent);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(tileView, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(tileView, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0,
                            LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0,
                            LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);
    return tileView;
}

void *GuiCreateAnimView(lv_obj_t *parent, uint16_t animHeight)
{
    lv_obj_t *cont =
        GuiCreateContainerWithParent(parent, 480, 800 - GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *img = GuiCreateImg(cont, &imgRing);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, animHeight - 60);

    img = GuiCreateImg(cont, &imgCircular);
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
    return cont;
}

void *GuiCreateArc(lv_obj_t *parent)
{
    lv_obj_t *arc = lv_arc_create(parent);
    lv_obj_set_size(arc, 120, 120);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_value(arc, 0);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(arc, WHITE_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(arc, LV_OPA_0, LV_PART_MAIN);

    return arc;
}

void *GuiCreateSwitch(lv_obj_t *parent)
{
    lv_obj_t *switchObj = lv_switch_create(parent);
    lv_obj_set_style_bg_color(switchObj, ORANGE_COLOR,
                              LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(switchObj, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(switchObj, LV_OPA_30, LV_PART_MAIN);

    return switchObj;
}

void GuiAlignToPrevObj(lv_obj_t *obj, lv_align_t align, int16_t x, int16_t y)
{
    lv_obj_align_to(obj, lv_obj_get_child(lv_obj_get_parent(obj), lv_obj_get_child_cnt(lv_obj_get_parent(obj)) - 2), align, x, y);
}

void GuiAddObjFlag(void *obj, lv_obj_flag_t flag)
{
    lv_obj_add_flag(obj, flag);
    if (flag & LV_OBJ_FLAG_SCROLLABLE) {
        lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ELASTIC);
    }

    if (flag & LV_OBJ_FLAG_CLICKABLE) {
        lv_obj_set_style_bg_opa(obj, LV_OPA_100, LV_STATE_DEFAULT);
    }
}

void GuiClearObjFlag(void *obj, lv_obj_flag_t flag)
{
    lv_obj_clear_flag(obj, flag);
    if (flag & LV_OBJ_FLAG_CLICKABLE) {
        lv_obj_set_style_bg_opa(obj, LV_OPA_60, LV_STATE_DEFAULT);
    }
}

void *GuiCreateSpacer(void *parent, uint16_t height)
{
    lv_obj_t *spacer =
        GuiCreateContainerWithParent(parent, lv_obj_get_width(parent), height);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_0, LV_PART_MAIN);
    return spacer;
}