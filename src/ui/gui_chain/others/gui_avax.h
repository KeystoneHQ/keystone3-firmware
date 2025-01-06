#ifndef _GUI_AVAX_H
#define _GUI_AVAX_H

#include "rust.h"
#include "stdbool.h"
#include "gui.h"

void GuiSetAvaxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetAvaxGUIData(void);
PtrT_TransactionCheckResult GuiGetAvaxCheckResult(void);
UREncodeResult *GuiGetAvaxSignQrCodeData(void);
UREncodeResult *GuiGetAvaxSignUrDataUnlimited(void);
void GuiAvaxTxOverview(lv_obj_t *parent, void *totalData);
void GuiAvaxTxRawData(lv_obj_t *parent, void *totalData);

#endif
