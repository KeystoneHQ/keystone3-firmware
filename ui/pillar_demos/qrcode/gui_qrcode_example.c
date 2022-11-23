/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_qrcode_example.c
 * Description:
 * author     : stone wang
 * data       : 2022-12-14 11:42
**********************************************************************/
#include "gui_qrcode_example.h"
#include "gui.h"

lv_obj_t *g_qrCodePage = NULL;

static void *gui_create_container(void)
{
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_set_style_bg_color(cont, GRAY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    return cont;
}

void lv_demo_qrcode(const uint8_t *data, uint32_t dataLen)
{
    g_qrCodePage = gui_create_container();

    lv_obj_t *qrcode = lv_qrcode_create(g_qrCodePage, lv_obj_get_width(lv_scr_act()), BLACK_COLOR, WHITE_COLOR);
    lv_qrcode_update(qrcode, data, dataLen);
    lv_obj_align(qrcode, LV_ALIGN_CENTER, 0, 0);
}
