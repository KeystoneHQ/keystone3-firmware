#ifndef _GUI_MULTI_ACCOUNTS_RECEIVE_WIDGETS_H
#define _GUI_MULTI_ACCOUNTS_RECEIVE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"
#include "gui_change_path_type_widgets.h"

void GuiMultiAccountsReceiveInit(uint8_t chain);
void GuiMultiAccountsReceiveDeInit(void);
void GuiMultiAccountsReceiveRefresh(void);
void GuiMultiAccountsReceivePrevTile(void);
void GuiResetCurrentStandardAddressIndex(void);
void GuiResetAllStandardAddressIndex(void);
#endif
