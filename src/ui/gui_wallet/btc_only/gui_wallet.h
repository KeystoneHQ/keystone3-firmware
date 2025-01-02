#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"
#include "gui_chain.h"
#include "rsa.h"
#include "gui_attention_hintbox.h"

UREncodeResult *GuiGetBlueWalletBtcData(void);
UREncodeResult *GuiGetSparrowWalletBtcData(void);
UREncodeResult *GuiGetSpecterWalletBtcData(void);
#ifndef BTC_ONLY

#ifndef COMPILE_SIMULATOR
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType);
#endif
#endif
#ifndef BTC_ONLY

#endif
#endif
