#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"
#include "gui_chain.h"

UREncodeResult *GuiGetBlueWalletBtcData(void);
UREncodeResult *GuiGetKeplrData(void);
UREncodeResult *GuiGetCompanionAppData(void);
#ifndef COMPILE_SIMULATOR
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType);
#endif
UREncodeResult *GuiGetMetamaskData(void);
UREncodeResult *GuiGetOkxWalletData(void);
UREncodeResult *GuiGetFewchaDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetPetraData(void);
UREncodeResult *GuiGetSolflareData(void);
UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index);
UREncodeResult *GuiGetImTokenData(void);
UREncodeResult *GuiGetSenderDataByIndex(uint16_t index);
#endif
