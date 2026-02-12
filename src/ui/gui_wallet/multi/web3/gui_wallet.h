#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"
#include "gui_chain.h"
#include "rsa.h"
#include "gui_attention_hintbox.h"

UREncodeResult *GuiGetStandardBtcData(void);
UREncodeResult *GuiGetKeplrDataByIndex(uint32_t index);
UREncodeResult *GuiGetLeapData(void);
UREncodeResult *GuiGetWanderData(void);
UREncodeResult *GuiGetCompanionAppData(void);
UREncodeResult *GuiGetOkxWalletData(void);
UREncodeResult *GuiGetBitgetWalletData(void);
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GuiGetMetamaskData(void);
UREncodeResult *GuiGetWalletDataByCoin(bool onlySui);
UREncodeResult *GuiGetFewchaDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetNightlyDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetIotaWalletData(void);
UREncodeResult *GuiGetPetraData(void);
UREncodeResult *GuiGetSolflareData(void);
UREncodeResult *GuiGetHeliumData(void);
UREncodeResult *GuiGetXBullData(void);
UREncodeResult *GuiGetBackpackData(void);
UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index);
UREncodeResult *GuiGetADADataByIndex(char *walletName);
UREncodeResult *GuiGetImTokenData(void);
UREncodeResult *GuiGetCoreWalletData(void);
UREncodeResult *GuiGetThorWalletData(void);
UREncodeResult *GuiGetKaspiumData(void);
UREncodeResult *GuiGetKeystoneConnectWalletDataBip39(void);
UREncodeResult *GuiGetKeystoneConnectWalletDataSlip39(void);
#endif
