#include "gui_chain_components.h"
#include <stdint.h>
#include <string.h>
#include "user_memory.h"

#define GUI_PAGED_MESSAGE_BYTES 512
#define GUI_PAGED_MESSAGE_BREAK_SEARCH_BYTES 128

typedef struct {
    GuiPagedMessageSource_t source;
    size_t offset;
    size_t page;
    size_t page_count;
    lv_obj_t *viewport;
    lv_obj_t *warning;
    lv_obj_t *label;
    lv_obj_t *page_label;
    lv_obj_t *prev;
    lv_obj_t *next;
} GuiPagedMessageState_t;

const lv_font_t *GetOverviewAmountFont(const char *value)
{
    size_t length = strlen(value);
    if (length <= 24) {
        return g_defLittleTitleFont;
    }
    if (length <= 40) {
        return g_defTextFont;
    }
    return g_defIllustrateFont;
}

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
    return CreateTransactionItemViewWithHintAndWidth(parent, title, value, lastView, hint, 408);
}

lv_obj_t *CreateTransactionItemViewWithHintAndWidth(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView, const char* hint, uint16_t width)
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
    lv_obj_t *container = CreateTransactionContentContainer(parent, width, height);
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
    bool overflow = totalWidth > width || valueHeight > 30;
    if (!overflow) {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    } else {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_set_width(valueLabel, width - 48);
        lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(valueLabel);

        height += lv_obj_get_height(valueLabel);

        lv_obj_set_height(container, height);
    }

    if (hint != NULL) {
        lv_obj_t *hintLabel = GuiCreateIllustrateLabel(container, hint);
        lv_obj_align_to(hintLabel, valueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        height += 4;
        lv_obj_set_width(hintLabel, width - 48);
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

lv_obj_t *CreateTransactionItemViewWithWidth(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView, uint16_t width)
{
    return CreateTransactionItemViewWithHintAndWidth(parent, title, value, lastView, NULL, width);
}

lv_obj_t *CreateTransactionOvewviewCard(lv_obj_t *parent, const char* title1, const char* text1, const char* title2, const char* text2)
{
    return CreateTransactionOverviewCardWithWidth(parent, title1, text1, title2, text2, 408);
}

lv_obj_t *CreateTransactionOverviewCardWithWidth(lv_obj_t *parent, const char* title1, const char* text1, const char* title2, const char* text2, uint16_t width)
{
    uint16_t height = 16;//top padding
    lv_obj_t *container = CreateTransactionContentContainer(parent, width, 0);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 4);

    lv_obj_t *label;
    label = GuiCreateIllustrateLabel(container, title1);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    height += 30 + 4;
    label = GuiCreateLabelWithFont(container, text1, GetOverviewAmountFont(text1));
    lv_obj_set_width(label, width - 48);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
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
    bool overflow = totalWidth > width || valueHeight > 30;

    height += 30; //title height;

    if (!overflow) {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    } else {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_set_width(valueLabel, width - 48);
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
        lv_obj_t *feeKeyLabel = GuiCreateIllustrateLabel(container, feeKey);
        lv_obj_align(feeKeyLabel, LV_ALIGN_TOP_LEFT, 24, 98);
        lv_obj_set_style_text_opa(feeKeyLabel, LV_OPA_64, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(container, fee);
        lv_obj_align_to(label, feeKeyLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
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

lv_obj_t *CreateNoticeCard(lv_obj_t *parent, const char *notice)
{
    return CreateNoticeCardWithWidth(parent, notice, 408);
}

lv_obj_t *CreateNoticeCardWithWidth(lv_obj_t *parent, const char *notice, uint16_t width)
{
    return CreateTitledNoticeCard(parent, "Notice", notice, width);
}

lv_obj_t *CreateTitledNoticeCard(lv_obj_t *parent, const char *title, const char *notice, uint16_t width)
{
    uint16_t height = 24 + 36 + 8 + 24;
    lv_obj_t* card = GuiCreateContainerWithParent(parent, width, 24);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(card, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card, 30, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* noticeIcon = GuiCreateImg(card, &imgNotice);
    lv_obj_align(noticeIcon, LV_ALIGN_TOP_LEFT, 24, 24);

    lv_obj_t* title_label = GuiCreateTextLabel(card, title);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(title_label, noticeIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t* content_label = GuiCreateIllustrateLabel(card, notice);
    lv_obj_set_width(content_label, width - 48);
    lv_obj_update_layout(content_label);
    height += lv_obj_get_self_height(content_label);
    lv_obj_set_height(card, height);
    lv_obj_align(content_label, LV_ALIGN_TOP_LEFT, 24, 68);

    return card;
}

void GuiCustomPathNotice(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 182);
    CreateNoticeCard(parent, _("custom_path_parse_notice"));
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
    lv_obj_set_width(label, width - 48);

    lv_obj_update_layout(label);
    uint16_t needed = 68 + lv_obj_get_self_height(label) + 24;
    if (needed > height) {
        lv_obj_set_height(noticeContainer, needed);
    }

    return noticeContainer;
}

static char GuiPagedMessageTextByteAt(void *context, size_t offset)
{
    return ((const char *)context)[offset];
}

static bool GuiPagedMessageIsUtf8Continuation(char value)
{
    return (((uint8_t)value) & 0xC0) == 0x80;
}

static size_t GuiPagedMessageFindBreak(const GuiPagedMessageState_t *state,
                                       size_t offset, size_t length, bool newlineOnly)
{
    size_t minimum = length > GUI_PAGED_MESSAGE_BREAK_SEARCH_BYTES
        ? length - GUI_PAGED_MESSAGE_BREAK_SEARCH_BYTES
        : 0;
    for (size_t i = length; i > minimum; i--) {
        char value = state->source.byte_at(state->source.context, offset + i - 1);
        if (value == '\n' || (!newlineOnly && (value == ' ' || value == '\t'))) {
            return i;
        }
    }
    return 0;
}

static size_t GuiPagedMessagePageEnd(const GuiPagedMessageState_t *state, size_t offset)
{
    if (offset >= state->source.length) {
        return state->source.length;
    }
    size_t remaining = state->source.length - offset;
    size_t length = remaining < GUI_PAGED_MESSAGE_BYTES
        ? remaining
        : GUI_PAGED_MESSAGE_BYTES;
    size_t byteLimit = length;

    if (offset + length < state->source.length && state->source.utf8) {
        size_t semanticBreak = GuiPagedMessageFindBreak(state, offset, length, true);
        if (semanticBreak == 0) {
            semanticBreak = GuiPagedMessageFindBreak(state, offset, length, false);
        }
        if (semanticBreak > 0) {
            length = semanticBreak;
        }
        while (length > 0 && GuiPagedMessageIsUtf8Continuation(
                   state->source.byte_at(state->source.context, offset + length))) {
            length--;
        }
        if (length == 0) {
            length = byteLimit;
        }
    }
    return offset + length;
}

static size_t GuiPagedMessagePageOffset(const GuiPagedMessageState_t *state, size_t page)
{
    size_t offset = 0;
    for (size_t i = 0; i < page && offset < state->source.length; i++) {
        offset = GuiPagedMessagePageEnd(state, offset);
    }
    return offset;
}

static void GuiPagedMessageRefresh(GuiPagedMessageState_t *state)
{
    char pageText[GUI_PAGED_MESSAGE_BYTES + 1];
    size_t end = GuiPagedMessagePageEnd(state, state->offset);
    size_t length = end - state->offset;
    for (size_t i = 0; i < length; i++) {
        pageText[i] = state->source.byte_at(state->source.context, state->offset + i);
    }
    pageText[length] = '\0';

    lv_coord_t labelY = 0;
    if (state->warning != NULL) {
        if (state->page == 0) {
            lv_obj_clear_flag(state->warning, LV_OBJ_FLAG_HIDDEN);
            lv_obj_update_layout(state->warning);
            labelY = lv_obj_get_height(state->warning) + 16;
        } else {
            lv_obj_add_flag(state->warning, LV_OBJ_FLAG_HIDDEN);
        }
    }
    lv_obj_set_y(state->label, labelY);
    lv_label_set_text(state->label, pageText);
    lv_obj_set_height(state->label, LV_SIZE_CONTENT);
    lv_obj_update_layout(state->viewport);
    lv_obj_scroll_to_y(state->viewport, 0, LV_ANIM_OFF);
    lv_label_set_text_fmt(state->page_label, "%u / %u",
                          (unsigned)(state->page + 1),
                          (unsigned)state->page_count);

    if (state->page == 0) {
        lv_obj_add_state(state->prev, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->prev, LV_STATE_DISABLED);
    }
    if (state->page + 1 >= state->page_count) {
        lv_obj_add_state(state->next, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->next, LV_STATE_DISABLED);
    }
}

static void GuiPagedMessageEvent(lv_event_t *event)
{
    GuiPagedMessageState_t *state = lv_event_get_user_data(event);
    lv_obj_t *target = lv_event_get_target(event);
    if (target == state->prev && state->page > 0) {
        state->page--;
    } else if (target == state->next && state->page + 1 < state->page_count) {
        state->page++;
    } else {
        return;
    }
    state->offset = GuiPagedMessagePageOffset(state, state->page);
    GuiPagedMessageRefresh(state);
}

static void GuiPagedMessageDelete(lv_event_t *event)
{
    GuiPagedMessageState_t *state = lv_event_get_user_data(event);
    SRAM_FREE(state);
}

static lv_obj_t *GuiPagedMessageButton(lv_obj_t *parent, const char *text)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, 72, 48);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, 30, LV_PART_MAIN);
    lv_obj_t *label = lv_label_create(button);
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *GuiPagedMessageWarning(lv_obj_t *parent, const char *titleText,
                                        const char *contentText, lv_coord_t width)
{
    lv_obj_t *warning = lv_obj_create(parent);
    lv_obj_set_width(warning, width);
    lv_obj_set_height(warning, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(warning, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_row(warning, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(warning, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(warning, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(warning, lv_color_hex(0xF55831), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(warning, 48, LV_PART_MAIN);
    lv_obj_set_flex_flow(warning, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title = GuiCreateTextLabel(warning, titleText);
    lv_obj_set_width(title, width - 32);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF55831), LV_PART_MAIN);

    lv_obj_t *content = GuiCreateIllustrateLabel(warning, contentText);
    lv_obj_set_width(content, width - 32);
    lv_label_set_long_mode(content, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(content, WHITE_COLOR, LV_PART_MAIN);
    return warning;
}

void GuiShowPagedMessage(lv_obj_t *parent, const GuiPagedMessageSource_t *source)
{
    if (parent == NULL || source == NULL || source->byte_at == NULL) {
        return;
    }

    GuiPagedMessageState_t *state = SRAM_MALLOC(sizeof(GuiPagedMessageState_t));
    if (state == NULL) {
        return;
    }
    memset(state, 0, sizeof(GuiPagedMessageState_t));
    state->source = *source;

    size_t offset = 0;
    do {
        state->page_count++;
        offset = GuiPagedMessagePageEnd(state, offset);
    } while (offset < state->source.length);

    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_update_layout(parent);
    lv_coord_t contentWidth = lv_obj_get_width(parent);
    lv_coord_t parentHeight = lv_obj_get_height(parent);
    lv_coord_t viewportHeight = parentHeight > 64 ? parentHeight - 64 : parentHeight;

    state->viewport = lv_obj_create(parent);
    lv_obj_set_pos(state->viewport, 0, 0);
    lv_obj_set_size(state->viewport, contentWidth, viewportHeight);
    lv_obj_set_style_pad_all(state->viewport, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(state->viewport, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(state->viewport, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(state->viewport, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(state->viewport, LV_DIR_VER);
    lv_obj_add_flag(state->viewport, LV_OBJ_FLAG_SCROLLABLE);

    if (source->warning_title != NULL && source->warning_content != NULL) {
        state->warning = GuiPagedMessageWarning(
            state->viewport, source->warning_title, source->warning_content, contentWidth);
        lv_obj_set_pos(state->warning, 0, 0);
    }

    state->label = lv_label_create(state->viewport);
    lv_obj_set_pos(state->label, 0, 0);
    lv_obj_set_width(state->label, contentWidth);
    lv_obj_set_height(state->label, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(state->label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(state->label, WHITE_COLOR, LV_PART_MAIN);
    lv_label_set_long_mode(state->label, LV_LABEL_LONG_WRAP);

    state->prev = GuiPagedMessageButton(parent, "<");
    lv_obj_align(state->prev, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    state->next = GuiPagedMessageButton(parent, ">");
    lv_obj_align(state->next, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    state->page_label = lv_label_create(parent);
    lv_obj_set_style_text_font(state->page_label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(state->page_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(state->page_label, LV_ALIGN_BOTTOM_MID, 0, -9);

    lv_obj_add_event_cb(state->prev, GuiPagedMessageEvent, LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(state->next, GuiPagedMessageEvent, LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(parent, GuiPagedMessageDelete, LV_EVENT_DELETE, state);
    GuiPagedMessageRefresh(state);
}

void GuiShowPagedMessageText(lv_obj_t *parent, const char *text, bool utf8,
                             const char *warningTitle, const char *warningContent)
{
    const char *safeText = text == NULL ? "" : text;
    GuiPagedMessageSource_t source = {
        .context = (void *)safeText,
        .length = strlen(safeText),
        .byte_at = GuiPagedMessageTextByteAt,
        .utf8 = utf8,
        .warning_title = warningTitle,
        .warning_content = warningContent,
    };
    GuiShowPagedMessage(parent, &source);
}
