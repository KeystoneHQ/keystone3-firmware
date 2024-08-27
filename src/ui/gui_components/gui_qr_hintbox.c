#include <stdio.h>
#include "gui.h"
#include "gui_obj.h"
#include "user_memory.h"
#include "gui_views.h"
#include "gui_qr_hintbox.h"
#include "gui_hintbox.h"

static lv_obj_t *g_qrHintBox = NULL;
static lv_obj_t *g_qrHintBoxTitle = NULL;
static lv_obj_t *g_qrHintBoxQR = NULL;
static lv_obj_t *g_qrHintBoxSubTitle = NULL;

void GuiQRHintBoxRemove();

static void CheckAndClearFlag(lv_obj_t *obj, lv_obj_flag_t flag)
{
    if (lv_obj_has_flag(obj, flag)) {
        lv_obj_clear_flag(obj, flag);
    }
}

// static void CheckAndAddFlag(lv_obj_t *obj, lv_obj_flag_t flag)
// {
//     if (!lv_obj_has_flag(obj, flag)) {
//         lv_obj_add_flag(obj, flag);
//     }
// }

static void GuiQRHintBoxCloseHandler(lv_event_t *e)
{
    GuiQRHintBoxRemove();
}

void GuiQRHintBoxRemove()
{
    GUI_DEL_OBJ(g_qrHintBox);
}

bool GuiQRHintBoxIsActive()
{
    return g_qrHintBox != NULL;
}


void GuiQRCodeHintBoxOpenBig(const char *qrdata, const char *title, const char *content, const char *url)
{
    lv_obj_t *button, *qrCodeCont, *qrCode;
    if (g_qrHintBox == NULL) {
        g_qrHintBox = GuiCreateHintBox(746);
        // qr code container
        qrCodeCont = lv_obj_create(g_qrHintBox);
        lv_obj_set_size(qrCodeCont, 408, 408);
        lv_obj_set_style_border_width(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(qrCodeCont, 0, 0);
        lv_obj_set_style_pad_all(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(qrCodeCont, 16, LV_PART_MAIN);
        lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(qrCodeCont, WHITE_COLOR, LV_PART_MAIN);
        // top mid to hint box
        lv_obj_align_to(qrCodeCont, g_qrHintBox, LV_ALIGN_TOP_MID, 0, 90);
        qrCode = lv_qrcode_create(qrCodeCont, 360, BLACK_COLOR, WHITE_COLOR);
        lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);
        lv_qrcode_update(qrCode, qrdata, (uint32_t)strlen(qrdata));
        g_qrHintBoxQR = qrCode;
        // create a scrollable container
        lv_obj_t *scrollable = lv_obj_create(g_qrHintBox);
        lv_obj_set_size(scrollable, 408, 220);
        lv_obj_set_style_bg_color(scrollable, DARK_BG_COLOR,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_scrollbar_mode(scrollable, LV_SCROLLBAR_MODE_OFF);
        // content align to qr code container
        lv_obj_align_to(scrollable, qrCodeCont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
        lv_obj_set_style_border_width(scrollable, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(scrollable, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        // title
        lv_obj_t * g_qrHintBoxTitle = GuiCreateTextLabel(scrollable, title);
        lv_label_set_long_mode(g_qrHintBoxTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_align_to(g_qrHintBoxTitle, qrCodeCont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        lv_obj_set_style_text_font(g_qrHintBoxTitle, &openSansEnTitle, LV_PART_MAIN);
        // content
        lv_obj_t * g_qrHintBoxSubTitle = GuiCreateIllustrateLabel(scrollable, content);
        lv_obj_set_style_text_color(g_qrHintBoxSubTitle, WHITE_COLOR, LV_PART_MAIN);
        if (strcmp(content, "") != 0) {
            lv_obj_align_to(g_qrHintBoxSubTitle, g_qrHintBoxTitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
        }
        // url
        lv_obj_t * urlLabel = GuiCreateIllustrateLabel(scrollable, url);
        lv_obj_set_style_text_color(urlLabel, BLUE_GREEN_COLOR, LV_PART_MAIN);
        lv_obj_align_to(urlLabel, g_qrHintBoxSubTitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
        g_qrHintBoxSubTitle = urlLabel;

        button = GuiCreateTextBtn(g_qrHintBox, _("OK"));
        lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(button, GuiQRHintBoxCloseHandler, LV_EVENT_CLICKED, NULL);
    } else {
        //enhancement
        CheckAndClearFlag(g_qrHintBox, LV_OBJ_FLAG_HIDDEN);
        lv_qrcode_update(g_qrHintBoxQR, qrdata, (uint32_t)strnlen_s(qrdata, 1024));
        lv_label_set_text(g_qrHintBoxTitle, title);
        lv_label_set_text(g_qrHintBoxSubTitle, content);
    }
}

void GuiNormalHitBoxOpen(const char *title, const char *content)
{
    if (g_qrHintBox == NULL) {
        // calculate the content height and create hintbox
        lv_coord_t max_width = 420;
        lv_point_t size;
        lv_txt_get_size(&size, content, &openSansEnIllustrate, 0, 0, max_width, LV_TEXT_FLAG_NONE);

        int lineCount = size.y / lv_font_get_line_height(&openSansEnIllustrate);

        // line height 30
        int lineHeight = 30;
        int hintBoxHeight = lineCount * lineHeight + 230;
        g_qrHintBox = GuiCreateHintBox(hintBoxHeight);

        // create title
        lv_obj_t * titleLabel = GuiCreateTextLabel(g_qrHintBox, title);
        lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_align_to(titleLabel, g_qrHintBox, LV_ALIGN_TOP_LEFT, 25, 800 - hintBoxHeight + 36);
        g_qrHintBoxTitle = titleLabel;

        // create scrollable container
        lv_obj_t *scrollable = lv_obj_create(g_qrHintBox);
        lv_obj_set_size(scrollable, 420, 202);
        lv_obj_set_style_bg_color(scrollable, DARK_BG_COLOR,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_scrollbar_mode(scrollable, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_border_width(scrollable, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(scrollable, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align_to(scrollable, g_qrHintBoxTitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

        lv_obj_t *label = GuiCreateIllustrateLabel(scrollable, content);
        lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
        g_qrHintBoxSubTitle = label;
        lv_obj_align_to(label, scrollable, LV_ALIGN_TOP_LEFT, 0, 0);

        lv_obj_t *button = GuiCreateTextBtn(g_qrHintBox, _("OK"));
        lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -24, -24);
        lv_obj_add_event_cb(button, GuiQRHintBoxCloseHandler, LV_EVENT_CLICKED, NULL);
    } else {
        //enhancement
        CheckAndClearFlag(g_qrHintBox, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(g_qrHintBoxTitle, title);
        lv_label_set_text(g_qrHintBoxSubTitle, content);
    }
}

void GuiQRCodeHintBoxOpen(const char *qrdata, const char *title, const char *subtitle)
{
    lv_obj_t *parent, *button, *qrCodeCont, *qrCode, *label;

    if (g_qrHintBox == NULL) {
        g_qrHintBox = GuiCreateHintBox(656);
        parent = g_qrHintBox;

        qrCodeCont = lv_obj_create(parent);
        lv_obj_set_size(qrCodeCont, 408, 408);
        lv_obj_set_style_border_width(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(qrCodeCont, 0, 0);
        lv_obj_set_style_pad_all(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(qrCodeCont, 16, LV_PART_MAIN);
        lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(qrCodeCont, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align(qrCodeCont, LV_ALIGN_BOTTOM_MID, 0, -220);

        qrCode = lv_qrcode_create(qrCodeCont, 360, BLACK_COLOR, WHITE_COLOR);
        lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);
        lv_qrcode_update(qrCode, qrdata, (uint32_t)strlen(qrdata));
        g_qrHintBoxQR = qrCode;

        label = GuiCreateTextLabel(parent, title);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -156);
        g_qrHintBoxTitle = label;

        label = GuiCreateIllustrateLabel(parent, subtitle);
        lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);
        GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 0, 40);
        g_qrHintBoxSubTitle = label;

        button = GuiCreateTextBtn(parent, _("OK"));
        lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(button, GuiQRHintBoxCloseHandler, LV_EVENT_CLICKED, NULL);
    } else {
        //enhancement
        CheckAndClearFlag(g_qrHintBox, LV_OBJ_FLAG_HIDDEN);
        lv_qrcode_update(g_qrHintBoxQR, qrdata, (uint32_t)strnlen_s(qrdata, 1024));
        lv_label_set_text(g_qrHintBoxTitle, title);
        lv_label_set_text(g_qrHintBoxSubTitle, subtitle);
    }
}