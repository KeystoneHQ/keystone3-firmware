#include <stdio.h>
#include "gui.h"
#include "gui_obj.h"
#include "user_memory.h"
#include "gui_views.h"
#include "gui_qr_hintbox.h"
#include "gui_hintbox.h"
#include "safe_str_lib.h"

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
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiQRHintBoxRemove();
    }
}

void GuiQRHintBoxRemove()
{
    GUI_DEL_OBJ(g_qrHintBox);
}

bool GuiQRHintBoxIsActive()
{
    return g_qrHintBox != NULL;
}

void GuiQRCodeHintBoxOpen(const char *qrdata, const char *title, const char *subtitle)
{
    lv_obj_t *parent, *button, *qrCodeCont, *qrCode, *label;

    if (g_qrHintBox == NULL) {
        g_qrHintBox = GuiCreateHintBox(lv_scr_act(), 480, 656, true);
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
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -114);
        g_qrHintBoxSubTitle = label;

        button = GuiCreateBtn(parent, _("OK"));
        lv_obj_set_size(button, 94, 66);
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