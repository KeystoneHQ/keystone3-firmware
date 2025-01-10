#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"
#include "gui_chain.h"

UREncodeResult *GuiGetBlueWalletBtcData(void);
UREncodeResult *GuiGetSparrowWalletBtcData(void);
UREncodeResult *GuiGetSpecterWalletBtcData(void);

#endif
