#include "../../lv_examples.h"
#include "gui.h"
#if LV_USE_TILEVIEW && LV_BUILD_EXAMPLES

/**
 * Create a 2x2 tile view and allow scrolling only in an "L" shape.
 * Demonstrate scroll chaining with a long list that
 * scrolls the tile view when it can't be scrolled further.
 */


#if 0
LV_IMG_DECLARE(Frame);
LV_IMG_DECLARE(cbtn);
LV_IMG_DECLARE(setlang);
LV_IMG_DECLARE(crtea);
#endif

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

static lv_obj_t *g_setUpPage = NULL;

static void *gui_create_container(void)
{
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    return cont;
}

void lv_example_tileview_1(void)
{
#if 0
    g_setUpPage = gui_create_container();
    lv_obj_t * tv = lv_tileview_create(g_setUpPage);

    /*Tile1: just a label*/
    lv_obj_t * tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_RIGHT);
    lv_obj_t *setUpimg = lv_img_create(tile1);
    lv_img_set_src(setUpimg, &Frame);
    lv_obj_set_align(setUpimg, LV_ALIGN_CENTER);

#if 0
    lv_obj_t *label = lv_label_create(tile1);
    lv_label_set_text(label, "");
    lv_obj_t *btnImg = lv_btn_create(tile1);
    lv_imgbtn_set_src(btnImg, LV_IMGBTN_STATE_RELEASED, &btn, &btn, &btn);
//    lv_img_set_src(btnImg, &btn);
    lv_obj_add_event_cb(btnImg, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btnImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(btnImg, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btnImg, LV_SIZE_CONTENT);
#endif
#if 0
    lv_label_set_text(label, "Scroll down");
    lv_obj_center(label);
#endif


    /*Tile2: a button*/
    lv_obj_t * tile2 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);

//    lv_obj_t * btn = lv_btn_create(tile2);

    lv_obj_t *setLangimg = lv_img_create(tile2);
    lv_img_set_src(setLangimg, &setlang);
    lv_obj_set_align(setLangimg, LV_ALIGN_CENTER);

#if 0
    lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(btn);
#endif

    /*Tile3: a list*/
    lv_obj_t * tile3 = lv_tileview_add_tile(tv, 2, 0, LV_DIR_LEFT);
    lv_obj_t *createimg = lv_img_create(tile3);
    lv_img_set_src(createimg, &crtea);
    lv_obj_set_align(createimg, LV_ALIGN_CENTER);
#endif

#if 0
    lv_obj_t * list = lv_list_create(tile3);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));

    lv_list_add_btn(list, NULL, "One");
    lv_list_add_btn(list, NULL, "Two");
    lv_list_add_btn(list, NULL, "Three");
    lv_list_add_btn(list, NULL, "Four");
    lv_list_add_btn(list, NULL, "Five");
    lv_list_add_btn(list, NULL, "Six");
    lv_list_add_btn(list, NULL, "Seven");
    lv_list_add_btn(list, NULL, "Eight");
    lv_list_add_btn(list, NULL, "Nine");
    lv_list_add_btn(list, NULL, "Ten");
#endif

}

#endif
