#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "gui.h"
#include "gui_hintbox.h"

static lv_obj_t *g_hintbox = NULL;

void gui_hintbox_example_1(void)
{
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    g_hintbox = GuiCreateHintBox(cont, 480, 560, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_hintbox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_hintbox);
}

