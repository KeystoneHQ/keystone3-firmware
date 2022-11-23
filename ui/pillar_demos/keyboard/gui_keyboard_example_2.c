/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_keyboard_example.c
 * Description:
 * author     : stone wang
 * data       : 2023-01-18 15:16
**********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "gui.h"
#include "gui_keyboard.h"
#include "gui_create_wallet_widgets.h"

static KeyBoard_t *g_keyBoard;
lv_obj_t *g_cont;

static void FullKbTestHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        int len = lv_textarea_get_cursor_pos(g_keyBoard->ta);
        printf("len = %d\n", len);
    }
}


void gui_keyboard_example_2(void)
{
    g_cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    g_keyBoard = GuiCreateFullKeyBoard(g_cont, FullKbTestHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_keyBoard, 1);
    lv_keyboard_set_popovers(g_keyBoard->kb, true);

    lv_obj_clear_flag(g_keyBoard->ta, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(g_keyBoard->ta, 400, 75);
    lv_obj_set_style_border_width(g_keyBoard->ta, 2, LV_PART_MAIN);
    lv_obj_set_style_text_opa(g_keyBoard->ta, LV_OPA_100, 0);
    lv_obj_align(g_keyBoard->ta, LV_ALIGN_DEFAULT, 0, 316 - GUI_MAIN_AREA_OFFSET);
    lv_textarea_set_placeholder_text(g_keyBoard->ta, "Wallet Name");
    lv_textarea_set_text_selection(g_keyBoard->ta, true);
    lv_textarea_set_max_length(g_keyBoard->ta, 128);
    GuiConfirmFullKeyBoard(g_keyBoard);

    lv_obj_set_style_bg_color(g_keyBoard->ta, ORANGE_COLOR, LV_PART_KNOB);
    lv_textarea_set_one_line(g_keyBoard->ta, true);
    printf("g_keyboard->mode = %d\n", g_keyBoard->mode);
//    GuiConfirmFullKeyBoard(g_keyBoard);
}
