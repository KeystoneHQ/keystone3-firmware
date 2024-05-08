#ifndef _GUI_AR_H
#define _GUI_AR_H

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "librust_c.h"
#include "gui_chain.h"
#include "gui_lock_widgets.h"
#include "screen_manager.h"
#include "account_public_info.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

PtrT_TransactionCheckResult GuiGetArCheckResult(void);
void GuiSetArUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetArData(void);
void FreeArMemory(void);
void GetArweaveValue(void *indata, void *param, uint32_t maxLen);
void GetArweaveFee(void *indata, void *param, uint32_t maxLen);
void GetArweaveFromAddress(void *indata, void *param, uint32_t maxLen);
void GetArweaveToAddress(void *indata, void *param, uint32_t maxLen);
void GuiShowArweaveTxDetail(lv_obj_t *parent, void *totalData);
void GetArweaveMessageText(void *indata, void *param, uint32_t maxLen);
void GetArweaveRawMessage(void *indata, void *param, uint32_t maxLen);
void GetArweaveMessageAddress(void *indata, void *param, uint32_t maxLen);
UREncodeResult *GuiGetArweaveSignQrCodeData(void);

#endif