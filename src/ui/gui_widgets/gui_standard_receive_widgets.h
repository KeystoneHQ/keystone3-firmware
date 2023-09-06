#ifndef _GUI_ETH_RECEIVE_WIDGETS_H
#define _GUI_ETH_RECEIVE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"

void GuiStandardReceiveInit(uint8_t chain);
void GuiStandardReceiveDeInit(void);
void GuiStandardReceiveRefresh(void);
void GuiStandardReceivePrevTile(void);
void GuiResetCurrentStandardAddressIndex(void);
void GuiResetAllStandardAddressIndex(void);
#endif
