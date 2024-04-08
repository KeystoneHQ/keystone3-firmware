#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"
#include "gui_chain.h"
#include "rsa.h"

UREncodeResult *GuiGetBlueWalletBtcData(void);
UREncodeResult *GuiGetSparrowWalletBtcData(void);
#ifndef BTC_ONLY
UREncodeResult *GuiGetKeplrData(void);
UREncodeResult *GuiGetArConnectData(void);
UREncodeResult *GuiGetCompanionAppData(void);
#ifndef COMPILE_SIMULATOR
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType);
#endif
UREncodeResult *GuiGetMetamaskData(void);
#endif
UREncodeResult *GuiGetOkxWalletData(void);
#ifndef BTC_ONLY
UREncodeResult *GuiGetFewchaDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetPetraData(void);
UREncodeResult *GuiGetSolflareData(void);
UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index);
UREncodeResult *GuiGetImTokenData(void);
#endif
#endif
