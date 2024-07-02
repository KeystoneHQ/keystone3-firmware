#include "gui_chain_components.h"

lv_obj_t *CreateTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, w, h);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(container, LV_OPA_12, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN);
    return container;
}

lv_obj_t *CreateTransactionItemView(lv_obj_t *parent, char* title, char* value, lv_obj_t *lastView)
{
    //basic style:
    // ______________________________
    //|#############16px#############|
    //|#24px#title#16px#value#24px###|
    //|#############16px#############|
    // ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔
    //text line height is 30
    //when value overflow one line, it should be:
    // ______________________________
    //|#############16px#############|
    //|#24px#title###################|
    //|#24px#value###################|
    //|#24px#value###################|
    //|#############16px#############|
    // ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

    //62 is the basic height = 16 + 30 + 16
    lv_obj_t *container = CreateTransactionContentContainer(parent, 408, 62);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    //render title
    lv_obj_t *titleLabel = GuiCreateIllustrateLabel(container, title);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
    lv_obj_update_layout(titleLabel);
    uint16_t titleWidth = lv_obj_get_width(titleLabel);

    //render value
    lv_obj_t *valueLabel = GuiCreateIllustrateLabel(container, value);
    lv_obj_update_layout(valueLabel);
    uint16_t valueWidth = lv_obj_get_width(valueLabel);
    uint16_t valueHeight = lv_obj_get_height(valueLabel);

    uint16_t totalWidth = 24+titleWidth+16+valueWidth+24;
    printf("valueWidth: %d\n", valueWidth);
    printf("valueHeight: %d\n", valueHeight);
    printf("totalWidth: %d\n", totalWidth);
    bool overflow = totalWidth > 408 || valueHeight > 30;
    if(!overflow) {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    }
    else {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_set_width(valueLabel, 360);
        lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(valueLabel);

        uint16_t height = lv_obj_get_height(valueLabel);

        lv_obj_set_height(container, height + 62);
        lv_obj_update_layout(container);
    }

    return container;
}