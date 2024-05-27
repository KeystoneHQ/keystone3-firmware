#ifndef _GUI_TON_H
#define _GUI_TON_H

#include "rust.h"
#include "stdbool.h"

void GuiSetTonUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetTonGUIData(void);
UREncodeResult *GuiGetTonSignQrCodeData(void);
PtrT_TransactionCheckResult GuiGetTonCheckResult(void);

#endif
