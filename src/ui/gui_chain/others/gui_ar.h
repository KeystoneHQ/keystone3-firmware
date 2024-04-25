#ifndef _GUI_AR_H
#define _GUI_AR_H

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "librust_c.h"
#include "gui_chain.h"

PtrT_TransactionCheckResult GuiGetArCheckResult(void);
void GuiSetArUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetArData(void);
void FreeArMemory(void);
UREncodeResult *GuiGetArweaveSignQrCodeData(void);

#endif