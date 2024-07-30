#ifndef _GUI_CHANGE_PATH_TYPE_H
#define _GUI_CHANGE_PATH_TYPE_H

#include "gui_chain.h"
#include "gui_obj.h"
#include "gui_framework.h"
#include "assert.h"
#include "gui_global_resources.h"

void GuiCreateSwitchPathTypeWidget(lv_obj_t *parent, HOME_WALLET_CARD_ENUM chain);
void GuiDestroySwitchPathTypeWidget(void);

#endif