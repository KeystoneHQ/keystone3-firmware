
#ifndef _GUI_MULTI_PATH_COIN_RECEIVE_WIDGETS_H
#define _GUI_MULTI_PATH_COIN_RECEIVE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"

void GuiMultiPathCoinReceiveInit(uint8_t chain);
void GuiMultiPathCoinReceiveDeInit(void);
void GuiMultiPathCoinReceiveRefresh(void);
void GuiMultiPathCoinReceivePrevTile(void);
void AddressLongModeCut(char *out, const char *address);
void GuiResetCurrentEthAddressIndex(void);
void GuiResetAllEthAddressIndex(void);
#endif

