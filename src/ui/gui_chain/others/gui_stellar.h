#ifndef _GUI_STELLAR_H
#define _GUI_STELLAR_H

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "librust_c.h"
#include "gui_views.h"
#include "gui_chain.h"

PtrT_TransactionCheckResult GuiGetStellarCheckResult(void);
void *GuiGetStellarData(void);
void FreeStellarMemory(void);
void GuiSetStellarUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void GetStellarRawMessage(void *indata, void *param, uint32_t maxLen);
UREncodeResult *GuiGetStellarSignQrCodeData(void);
int GetStellarRawMessageLength(void *param);
void GuiStellarTxNotice(lv_obj_t *parent, void *totalData);
void GuiStellarHashNotice(lv_obj_t *parent, void *totalData);

#endif