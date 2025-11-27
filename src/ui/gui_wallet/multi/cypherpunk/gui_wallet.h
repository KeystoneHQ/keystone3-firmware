#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"
#include "gui_chain.h"
#include "rsa.h"
#include "gui_attention_hintbox.h"

UREncodeResult *GuiGetStandardBtcData(void);
UREncodeResult *GuiGetCompanionAppData(void);
UREncodeResult *GuiGetBitgetWalletData(void);
UREncodeResult *GuiGetCakeData(void);
uint8_t *OpenPrivateQrMode(void);
void ClosePrivateQrMode(void);
bool IsPrivateQrMode(void);

#endif
