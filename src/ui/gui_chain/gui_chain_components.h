#ifndef _GUI_CHAIN_COMPONENTS_H
#define _GUI_CHAIN_COMPONENTS_H

#include "gui_obj.h"

lv_obj_t *CreateTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
lv_obj_t *CreateTransactionItemView(lv_obj_t *parent, char* title, char* value, lv_obj_t *lastView);
lv_obj_t *CreateTransactionItemViewWithHint(lv_obj_t *parent, char* title, char* value, lv_obj_t *lastView, char* hint);
lv_obj_t *CreateValueOverviewValue(lv_obj_t *parent, char* value, char *fee);
lv_obj_t *CreateSingleInfoView(lv_obj_t *parent, char* key, char *value);
lv_obj_t *CreateContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
lv_obj_t *CreateValueDetailValue(lv_obj_t *parent, char* inputValue, char *outputValue, char *fee);
lv_obj_t *CreateDynamicInfoView(lv_obj_t *parent, char *key[], char *value[], int keyLen);

#endif