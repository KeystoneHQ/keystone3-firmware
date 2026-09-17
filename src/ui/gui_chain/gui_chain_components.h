#ifndef _GUI_CHAIN_COMPONENTS_H
#define _GUI_CHAIN_COMPONENTS_H

#include <stdbool.h>
#include <stddef.h>
#include "gui_obj.h"

typedef char (*GuiPagedMessageByteAtFunc)(void *context, size_t offset);

typedef struct {
    void *context;
    size_t length;
    GuiPagedMessageByteAtFunc byte_at;
    bool utf8;
    const char *warning_title;
    const char *warning_content;
} GuiPagedMessageSource_t;

lv_obj_t *CreateTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
lv_obj_t* CreateRelativeTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h, lv_obj_t *last_view);
lv_obj_t *CreateTransactionItemView(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView);
lv_obj_t *CreateTransactionItemViewWithWidth(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView, uint16_t width);
const lv_font_t *GetOverviewAmountFont(const char *value);
lv_obj_t *CreateTransactionItemViewWithHint(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView, const char* hint);
lv_obj_t *CreateTransactionItemViewWithHintAndWidth(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView, const char* hint, uint16_t width);
lv_obj_t *CreateValueOverviewValue(lv_obj_t *parent, const char *valueKey, const char *value,
                                   const char *feeKey, const char *fee);
lv_obj_t *CreateSingleInfoView(lv_obj_t *parent, char* key, char *value);
lv_obj_t *CreateContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
lv_obj_t *CreateValueDetailValue(lv_obj_t *parent, char* inputValue, char *outputValue, char *fee);
lv_obj_t *CreateDynamicInfoView(lv_obj_t *parent, char *key[], char *value[], int keyLen);
lv_obj_t *CreateNoticeCard(lv_obj_t *parent, const char* notice);
lv_obj_t *CreateNoticeCardWithWidth(lv_obj_t *parent, const char *notice, uint16_t width);
lv_obj_t *CreateTitledNoticeCard(lv_obj_t *parent, const char *title, const char *notice, uint16_t width);
lv_obj_t *CreateSingleInfoTwoLineView(lv_obj_t *parent, char* key, char *value);
lv_obj_t *CreateTransactionOvewviewCard(lv_obj_t *parent, const char* title1, const char* text1, const char* title2, const char* text2);
lv_obj_t *CreateTransactionOverviewCardWithWidth(lv_obj_t *parent, const char* title1, const char* text1, const char* title2, const char* text2, uint16_t width);
lv_obj_t *CreateNoticeView(lv_obj_t *parent, uint16_t width, uint16_t height, const char *notice);
void GuiCustomPathNotice(lv_obj_t *parent, void *totalData);
void GuiShowPagedMessage(lv_obj_t *parent, const GuiPagedMessageSource_t *source);
void GuiShowPagedMessageText(lv_obj_t *parent, const char *text, bool utf8,
                             const char *warning_title, const char *warning_content);

#endif
