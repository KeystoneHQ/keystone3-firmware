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

static lv_obj_t *g_cont1 = NULL;

void gui_scroll_snapshot_example_1(void)
{
    g_cont1 = GuiCreateContainer(480, 800);
    char mnemonicList[24][10];
    for (int i = 0; i < 24; i++) {
        sprintf(mnemonicList[i], "button%d", i);
    }
#if 0
    lv_obj_t *label = GuiCreateLabel(g_cont1, "g_cont1");
    lv_obj_set_align(label, LV_ALIGN_TOP_RIGHT);
    for (int i = 0; i < 16; i++) {
        sprintf(buf, "button%d", i);
        lv_obj_t *btn = GuiCreateBtn(g_cont1, buf);
        lv_obj_set_size(btn, 200, 50);
        lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 20 + i * 60);
    }
#endif
    KeyBoard_t *g_keyBoard = GuiCreateMnemonicKeyBoard(g_cont1, NULL,
                             KEY_STONE_MNEMONIC_24, mnemonicList);
    // GuiCreateScrollSnapShot(g_keyBoard->kb);

#if 0
    lv_obj_t *g_cont2 = GuiCreateContainer(480, 1000);
    lv_obj_set_style_bg_opa(g_cont2, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_cont2, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    g_testImg = GuiCreateImg(g_cont2, NULL);
    lv_obj_align(g_testImg, LV_ALIGN_DEFAULT, 0, 0);
    label = GuiCreateLabel(g_cont2, "g_cont2");
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_add_flag(g_cont2, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_event_cb(g_cont2, ReleaseHandler, LV_EVENT_ALL, NULL);
#endif

#if 0
    lv_obj_t *btn = GuiCreateBtn(cont2, "snap");
    lv_obj_set_align(btn, LV_ALIGN_BOTTOM_MID);

    lv_obj_t *img = GuiCreateImg(cont2, &emojiHappy);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 10, 200);
    lv_img_set_pivot(img, 0, 0);    /*Rotate around the top left corner*/

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, ImgSetX);
    lv_anim_set_values(&a, 10, 220);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_count(&a, 1);
    lv_anim_start(&a);

#endif
}

