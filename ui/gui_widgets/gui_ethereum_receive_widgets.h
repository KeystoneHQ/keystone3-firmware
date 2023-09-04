
#ifndef _GUI_ETH_RECEIVE_WIDGETS_H
#define _GUI_ETH_RECEIVE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"

void GuiEthereumReceiveInit(void);
void GuiEthereumReceiveDeInit(void);
void GuiEthereumReceiveRefresh(void);
void GuiEthereumReceivePrevTile(void);
void AddressLongModeCut(char *out, const char *address);
void GuiResetCurrentEthAddressIndex(void);
void GuiResetAllEthAddressIndex(void);
#endif

