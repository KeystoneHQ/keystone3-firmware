#ifndef _GUI_MONERO_H
#define _GUI_MONERO_H

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "librust_c.h"
#include "gui_views.h"
#include "gui_chain.h"
#include "log_print.h"

typedef enum XmrRequestType {
    OutputRequest = 1,
    UnsignedTxRequest,
} XmrRequestType;

PtrT_TransactionCheckResult GuiGetMoneroOutputCheckResult(void);
PtrT_TransactionCheckResult GuiGetMoneroUnsignedTxCheckResult(void);
void *GuiGetMoneroOutputData(void);
void *GuiGetMoneroUnsignedTxData(void);
void FreeMoneroMemory(void);
void GuiSetMoneroUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
UREncodeResult *GuiGetMoneroKeyimagesQrCodeData(void);
UREncodeResult *GuiGetMoneroSignedTransactionQrCodeData(void);
void GetXmrTxoCount(void *indata, void *param, uint32_t maxLen);
void GetXmrTotalAmount(void *indata, void *param, uint32_t maxLen);

#endif