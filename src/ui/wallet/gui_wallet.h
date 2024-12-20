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
UREncodeResult *GuiGetKeplrDataByIndex(uint32_t index);
UREncodeResult *GuiGetLeapData();
UREncodeResult *GuiGetArConnectData(void);
UREncodeResult *GuiGetCompanionAppData(void);
UREncodeResult *GuiGetOkxWalletData(void);
UREncodeResult *GuiGetBitgetWalletData(void);
#ifndef COMPILE_SIMULATOR
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType);
UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType);
#endif
UREncodeResult *GuiGetMetamaskData(void);
#endif
#ifndef BTC_ONLY
UREncodeResult *GuiGetFewchaDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetNightlyDataByCoin(GuiChainCoinType coin);
UREncodeResult *GuiGetPetraData(void);
UREncodeResult *GuiGetSolflareData(void);
UREncodeResult *GuiGetHeliumData(void);
UREncodeResult *GuiGetXBullData(void);
UREncodeResult *GuiGetBackpackData(void);
UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index);
UREncodeResult *GuiGetADADataByIndex(char *walletName);
UREncodeResult *GuiGetCakeData(void);
uint8_t *OpenPrivateQrMode(void);
void ClosePrivateQrMode(void);
bool IsPrivateQrMode(void);
uint8_t *GenerateCakeWalletEncryptPincode(void);
UREncodeResult *GuiGetImTokenData(void);
UREncodeResult *GuiGetKeystoneWalletData(void);
UREncodeResult *GuiGetThorWalletBtcData(void);
#endif
#endif
