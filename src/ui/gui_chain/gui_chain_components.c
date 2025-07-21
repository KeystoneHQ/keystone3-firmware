#include "gui_chain_components.h"

lv_obj_t* CreateRelativeTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h, lv_obj_t *last_view)
{
    lv_obj_t *container = CreateTransactionContentContainer(parent, w, h);
    if (last_view != NULL) {
        lv_obj_align_to(container, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }
    return container;
}

lv_obj_t *CreateTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, w, h);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(container, LV_OPA_12, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN);
    return container;
}

lv_obj_t *CreateTransactionItemViewWithHint(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView, const char* hint)
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
    uint16_t height = 62;
    lv_obj_t *container = CreateTransactionContentContainer(parent, 408, height);
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

    uint16_t totalWidth = 24 + titleWidth + 16 + valueWidth + 24;
    bool overflow = totalWidth > 408 || valueHeight > 30;
    if (!overflow) {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    } else {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_set_width(valueLabel, 360);
        lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(valueLabel);

        height += lv_obj_get_height(valueLabel);

        lv_obj_set_height(container, height);
    }

    if (hint != NULL) {
        lv_obj_t *hintLabel = GuiCreateIllustrateLabel(container, hint);
        lv_obj_align_to(hintLabel, valueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        height += 4;
        lv_obj_set_width(hintLabel, 360);
        lv_obj_set_style_text_color(hintLabel, ORANGE_COLOR, LV_PART_MAIN);
        lv_label_set_long_mode(hintLabel, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(hintLabel);

        height += lv_obj_get_height(hintLabel);

        lv_obj_set_height(container, height);
    }
    lv_obj_update_layout(container);
    return container;
}

lv_obj_t *CreateTransactionItemView(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView)
{
    return CreateTransactionItemViewWithHint(parent, title, value, lastView, NULL);
}

lv_obj_t *CreateTransactionOvewviewCard(lv_obj_t *parent, const char* title1, const char* text1, const char* title2, const char* text2)
{
    uint16_t height = 16;//top padding
    lv_obj_t *container = CreateTransactionContentContainer(parent, 408, 0);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 4);

    lv_obj_t *label;
    label = GuiCreateIllustrateLabel(container, title1);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    height += 30 + 4;
    label = GuiCreateLittleTitleLabel(container, text1);
    lv_obj_set_width(label, 360);
    lv_obj_update_layout(label);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);

    height += lv_obj_get_self_height(label) + 8;

    label = GuiCreateIllustrateLabel(container, title2);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    lv_obj_t *titleLabel = label;
    lv_obj_update_layout(label);
    uint16_t titleWidth = lv_obj_get_self_width(label);

    label = GuiCreateIllustrateLabel(container, text2);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_update_layout(label);
    lv_obj_t *valueLabel = label;

    uint16_t valueWidth = lv_obj_get_width(valueLabel);
    uint16_t valueHeight = lv_obj_get_height(valueLabel);

    uint16_t totalWidth = 24 + titleWidth + 16 + valueWidth + 24;
    bool overflow = totalWidth > 408 || valueHeight > 30;

    height += 30; //title height;

    if (!overflow) {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    } else {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_set_width(valueLabel, 360);
        lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(valueLabel);

        height += lv_obj_get_height(valueLabel);
    }

    height += 16;

    lv_obj_set_height(container, height);

    return container;
}

lv_obj_t *CreateValueOverviewValue(lv_obj_t *parent, const char *valueKey, const char *value,
                                   const char *feeKey, const char *fee)
{
    lv_obj_t *container = CreateContentContainer(parent, 408, feeKey == NULL ? 115 : 144);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, valueKey);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateTextLabel(container, value);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 50);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    if (feeKey != NULL) {
        label = GuiCreateIllustrateLabel(container, feeKey);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 98);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(container, fee);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 73, 98);
    }

    return container;
}

lv_obj_t *CreateValueDetailValue(lv_obj_t *parent, char* inputValue, char *outputValue, char *fee)
{
    lv_obj_t *container = CreateContentContainer(parent, 408, 138);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, _("Input Value"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, inputValue);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 147, 16);

    label = GuiCreateIllustrateLabel(container, _("Output Value"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, outputValue);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 164, 54);

    label = GuiCreateIllustrateLabel(container, _("Fee"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 92);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, fee);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 73, 92);

    return container;
}

lv_obj_t *CreateSingleInfoView(lv_obj_t *parent, char* key, char *value)
{
    return CreateDynamicInfoView(parent, &key, &value, 1);
}

lv_obj_t *CreateSingleInfoTwoLineView(lv_obj_t *parent, char* key, char *value)
{
    int height = 30 + 8 + 16 + 16;

    lv_obj_t *container = CreateContentContainer(parent, 408, height);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, _(key));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, value);
    lv_obj_set_width(label, 360);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_refr_size(label);

    lv_obj_set_height(container, height + lv_obj_get_self_height(label));

    return container;
}

lv_obj_t *CreateDynamicInfoView(lv_obj_t *parent, char *key[], char *value[], int keyLen)
{
    int height = (30 + 8) * keyLen - 8 + 16 + 16;

    lv_obj_t *container = CreateContentContainer(parent, 408, height);

    for (int i = 0; i < keyLen; i++) {
        lv_obj_t *label = GuiCreateIllustrateLabel(container, _(key[i]));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16 + 30 * i + 8 * i);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(container, value[i]);
        lv_obj_set_width(label, 300);
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
        lv_obj_refr_size(label);
        int labelHeight = lv_obj_get_self_height(label);
        height += labelHeight;
        lv_obj_set_height(container, height - 38);
    }

    return container;
}

lv_obj_t *CreateContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, w, h);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(container, LV_OPA_12, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN);
    return container;
}

lv_obj_t *CreateNoticeCard(lv_obj_t *parent, char *notice)
{
    uint16_t height = 24 + 36 + 8 + 24;
    lv_obj_t* card = GuiCreateContainerWithParent(parent, 408, 24);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(card, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card, 30, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* noticeIcon = GuiCreateImg(card, &imgNotice);
    lv_obj_align(noticeIcon, LV_ALIGN_TOP_LEFT, 24, 24);

    lv_obj_t* title_label = GuiCreateTextLabel(card, "Notice");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(title_label, noticeIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t* content_label = GuiCreateIllustrateLabel(card, notice);
    lv_obj_set_width(content_label, 360);
    lv_obj_update_layout(content_label);
    height += lv_obj_get_self_height(content_label);
    lv_obj_set_height(card, height);
    lv_obj_align(content_label, LV_ALIGN_TOP_LEFT, 24, 68);

    return card;
}

lv_obj_t *CreateNoticeView(lv_obj_t *parent, uint16_t width, uint16_t height, const char *notice)
{
    lv_obj_t *noticeContainer = GuiCreateContainerWithParent(parent, width, height);
    lv_obj_set_style_radius(noticeContainer, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(noticeContainer, RED_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(noticeContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *img = GuiCreateImg(noticeContainer, &imgNotice);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 24, 24);

    lv_obj_t *label = GuiCreateIllustrateLabel(noticeContainer, _("Notice"));
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    label = GuiCreateIllustrateLabel(noticeContainer, notice);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 68);
    lv_obj_set_width(label, 360);

    return noticeContainer;
}