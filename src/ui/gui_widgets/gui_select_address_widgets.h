#ifndef _GUI_SELECT_ADDRESS_WIDGETS_H
#define _GUI_SELECT_ADDRESS_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"
#include "gui.h"
#include "gui_chain.h"

typedef void (*SetSelectAddressIndexFunc)(uint32_t index);

lv_obj_t *GuiCreateSelectAddressWidget(GuiChainCoinType chainCoinType, uint32_t selectIndex, SetSelectAddressIndexFunc setIndex);
void GuiDestroySelectAddressWidget();

#endif
