#ifndef _GUI_SUI_H
#define _GUI_SUI_H

#include "rust.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_chain.h"

void GuiSetSuiUrData(void *data, bool multi);
void *GuiGetSuiData(void);
void FreeSuiMemory(void);
void GetSuiDetail(void *indata, void *param);

#endif
