/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Bitcoin receive address UI widgets.
 * Author: leon sun
 * Create: 2023-4-26
 ************************************************************************************************/

#ifndef _GUI_UTXO_RECEIVE_WIDGETS_H
#define _GUI_UTXO_RECEIVE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"

void GuiReceiveInit(uint8_t chain);
void GuiReceiveDeInit(void);
void GuiReceiveRefresh(void);
void GuiReceivePrevTile(void);
void GuiResetCurrentUtxoAddressIndex(void);
void GuiResetAllUtxoAddressIndex(void);
#endif

