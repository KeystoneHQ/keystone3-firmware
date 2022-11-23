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
#include "gui_button.h"

static void ButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_event_get_user_data(e));
    }
}

void gui_button_example_1(void)
{
    GuiButton_t table[2];
    GuiPosition_t position = {12, 12};
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_t *label1 = GuiCreateLabel(cont, "123");
    lv_obj_t *label2 = GuiCreateLabel(cont, "234");
    table[0].obj = label1;
    table[0].position = position;
    table[1].obj = label2;
    position.x = 200;
    table[1].position = position;
    lv_obj_t *button = GuiCreateButton(cont, 480, 100, table, 2, ButtonHandler, NULL);
}

