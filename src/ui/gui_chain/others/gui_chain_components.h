#ifndef _GUI_CHAIN_COMPONENTS_H
#define _GUI_CHAIN_COMPONENTS_H

#include "gui_obj.h"

lv_obj_t *CreateTransactionContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
lv_obj_t *CreateTransactionItemView(lv_obj_t *parent, char* title, char* value, lv_obj_t *lastView);

#endif