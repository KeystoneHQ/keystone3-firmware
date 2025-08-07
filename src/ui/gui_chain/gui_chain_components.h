#ifndef _GUI_CHAIN_COMPONENTS_H
#define _GUI_CHAIN_COMPONENTS_H

#include "gui_obj.h"

lv_obj_t *CreateTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
lv_obj_t* CreateRelativeTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h, lv_obj_t *last_view);
lv_obj_t *CreateTransactionItemView(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView);
lv_obj_t *CreateTransactionItemViewWithHint(lv_obj_t *parent, const char* title, const char* value, lv_obj_t *lastView, const char* hint);
lv_obj_t *CreateValueOverviewValue(lv_obj_t *parent, const char *valueKey, const char *value,
                                   const char *feeKey, const char *fee);
lv_obj_t *CreateSingleInfoView(lv_obj_t *parent, char* key, char *value);
lv_obj_t *CreateContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
lv_obj_t *CreateValueDetailValue(lv_obj_t *parent, char* inputValue, char *outputValue, char *fee);
lv_obj_t *CreateDynamicInfoView(lv_obj_t *parent, char *key[], char *value[], int keyLen);
lv_obj_t *CreateNoticeCard(lv_obj_t *parent, char* notice);
lv_obj_t *CreateSingleInfoTwoLineView(lv_obj_t *parent, char* key, char *value);
lv_obj_t *CreateTransactionOvewviewCard(lv_obj_t *parent, const char* title1, const char* text1, const char* title2, const char* text2);
lv_obj_t *CreateNoticeView(lv_obj_t *parent, uint16_t width, uint16_t height, const char *notice);
void GuiCustomPathNotice(lv_obj_t *parent, void *totalData);

#endif