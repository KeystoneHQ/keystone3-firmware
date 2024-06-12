#ifndef _GUI_ICP_H
#define _GUI_ICP_H

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "librust_c.h"
#include "gui_views.h"
#include "gui_chain.h"

PtrT_TransactionCheckResult GuiGetIcpCheckResult(void);
void *GuiGetIcpParseData(void);
void FreeIcpMemory(void);
void GuiSetIcpUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void GetIcpRawMessage(void *indata, void *param, uint32_t maxLen);
UREncodeResult *GuiGetIcpSignQrCodeData(void);
int GetIcpRawMessageLength(void *param);
void GuiIcpHashNotice(lv_obj_t *parent, void *totalData);

#endif