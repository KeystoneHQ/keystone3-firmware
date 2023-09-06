#include "../lv_examples.h"
#include "stdio.h"
#if LV_BUILD_EXAMPLES && LV_USE_FLEX


int array[30] = {0};

static void event_cb(lv_event_t * e)
{
    /*The original target of the event. Can be the buttons or the container*/
    lv_obj_t * target = lv_event_get_target(e);

    /*The current target is always the container as the event is added to it*/
    lv_obj_t * cont = lv_event_get_current_target(e);

    /*If container was clicked do nothing*/
    if (target == cont) return;

    /*Make the clicked buttons red*/
//    lv_label_get_text(cont);
    printf("target = %d\n", lv_obj_get_child_cnt(target));
    lv_label_t *label = lv_obj_get_child(target, 0);
    int id = 0;
    printf("text = %s\n", lv_label_get_text(label));
    sscanf(lv_label_get_text(label), &id);
    printf("id = %d\n", id);
    if (array[id] == 0) {
        lv_obj_set_style_bg_color(target, lv_palette_main(LV_PALETTE_RED), 0);
    } else {
        lv_obj_set_style_bg_color(target, lv_palette_main(LV_PALETTE_BLUE), 0);
    }
    array[id] = !array[id];
}

/**
 * Demonstrate event bubbling
 */
void lv_example_event_3(void)
{

    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 480, 480);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);

    uint32_t i;
    for (i = 0; i < 30; i++) {
        lv_obj_t * btn = lv_btn_create(cont);
        lv_obj_set_size(btn, 80, 50);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);

        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%"LV_PRIu32, i);
        lv_obj_center(label);
    }

    lv_obj_add_event_cb(cont, event_cb, LV_EVENT_CLICKED, NULL);
}

#endif
