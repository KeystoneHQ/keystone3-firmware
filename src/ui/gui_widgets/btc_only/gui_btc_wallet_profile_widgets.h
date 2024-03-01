#ifndef _GUI_BTC_WALLET_PROFILE_WIDGETS_H
#define _GUI_BTC_WALLET_PROFILE_WIDGETS_H

#include "stdint.h"

void GuiBtcWalletProfileInit(void);
void GuiBtcWalletProfileDeInit(void);
void GuiBtcWalletProfileRefresh(void);
int8_t GuiBtcWalletProfilePrevTile(uint8_t tileIndex);
int8_t GuiBtcWalletProfileNextTile(uint8_t tileIndex);


#endif
