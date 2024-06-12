#ifndef _GUI_TON_H
#define _GUI_TON_H

#include "rust.h"
#include "stdbool.h"
#include "gui.h"

void GuiSetTonUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetTonGUIData(void);
void *GuiGetTonProofGUIData(void);
UREncodeResult *GuiGetTonSignQrCodeData(void);
UREncodeResult *GuiGetTonProofSignQrCodeData(void);
PtrT_TransactionCheckResult GuiGetTonCheckResult(void);
void GuiTonTxOverview(lv_obj_t *parent, void *totalData);
void GuiTonTxRawData(lv_obj_t *parent, void *totalData);
void GuiTonProofOverview(lv_obj_t *parent, void *totalData);
void GuiTonProofRawData(lv_obj_t *parent, void *totalData);

#endif
