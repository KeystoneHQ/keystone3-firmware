#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "gui.h"
#include "gui_keyboard.h"
#include "gui_create_wallet_widgets.h"

static lv_obj_t *g_cont;

void gui_keyboard_example_5(void)
{
    g_cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_t *btnm = GuiCreateNumKeyboard(g_cont, NULL, NUM_KEYBOARD_PIN, NULL);
}
