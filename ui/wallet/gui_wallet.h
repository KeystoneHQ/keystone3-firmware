#ifndef GUI_WALLET_H
#define GUI_WALLET_H

#include "rust.h"

UREncodeResult *GuiGetBlueWalletBtcData(void);
UREncodeResult *GuiGetKeplrData(void);
UREncodeResult *GuiGetCompanionAppData(void);
UREncodeResult *GuiGetMetamaskData(void);
UREncodeResult *GuiGetOkxWalletData(void);

#endif
