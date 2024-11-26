#ifndef _GUI_ZCASH_H
#define _GUI_ZCASH_H
#include "rust.h"
#include "gui.h"

void *GuiGetZcashGUIData(void);

void GuiZcashOverview(lv_obj_t *parent, void *totalData);

#endif