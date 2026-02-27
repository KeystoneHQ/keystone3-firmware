#ifndef _GUI_UTXO_RECEIVE_WIDGETS_H
#define _GUI_UTXO_RECEIVE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"

void GuiReceiveInit(uint8_t chain);
void GuiReceiveDeInit(void);
void GuiReceiveRefresh(void);
void GuiReceivePrevTile(void);
void GuiResetCurrentUtxoAddressIndex(uint8_t index);
void GuiResetAllUtxoAddressIndex(void);
#endif

