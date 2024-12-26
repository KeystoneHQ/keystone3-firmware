#ifndef _GUI_SOL_H
#define _GUI_SOL_H

#include "rust.h"

void GuiSetSolUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetSolData(void);
UREncodeResult *GuiGetSolSignQrCodeData(void);
PtrT_TransactionCheckResult GuiGetSolCheckResult(void);
void *GuiGetSolMessageData(void);
void GetSolMessageType(void *indata, void *param, uint32_t maxLen);
void GetSolMessageFrom(void *indata, void *param, uint32_t maxLen);
void GetSolMessageUtf8(void *indata, void *param, uint32_t maxLen);
void GetSolMessageRaw(void *indata, void *param, uint32_t maxLen);

void FreeSolMemory(void);

void GuiShowSolTxOverview(lv_obj_t *parent, void *g_totalData);
void GuiShowSolTxDetail(lv_obj_t *parent, void *g_totalData);
#endif