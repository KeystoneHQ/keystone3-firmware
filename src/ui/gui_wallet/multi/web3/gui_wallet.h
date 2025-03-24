#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"
#include "gui_chain.h"
#include "rsa.h"
#include "gui_attention_hintbox.h"

UREncodeResult *GuiGetBlueWalletBtcData(void);
UREncodeResult *GuiGetSparrowWalletBtcData(void);
UREncodeResult *GuiGetSpecterWalletBtcData(void);
UREncodeResult *GuiGetKeplrDataByIndex(uint32_t index);
UREncodeResult *GuiGetLeapData();
UREncodeResult *GuiGetWanderData(void);
UREncodeResult *GuiGetCompanionAppData(void);
UREncodeResult *GuiGetOkxWalletData(void);
UREncodeResult *GuiGetBitgetWalletData(void);
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GuiGetMetamaskData(void);
UREncodeResult *GuiGetFewchaDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetNightlyDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetPetraData(void);
UREncodeResult *GuiGetSolflareData(void);
UREncodeResult *GuiGetHeliumData(void);
UREncodeResult *GuiGetXBullData(void);
UREncodeResult *GuiGetBackpackData(void);
UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index);
UREncodeResult *GuiGetADADataByIndex(char *walletName);
UREncodeResult *GuiGetImTokenData(void);
UREncodeResult *GuiGetCoreWalletData(void);
UREncodeResult *GuiGetThorWalletBtcData(void);
UREncodeResult *GuiGetKeystoneConnectWalletData(void);
#endif
