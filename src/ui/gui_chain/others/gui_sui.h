#ifndef _GUI_SUI_H
#define _GUI_SUI_H

void GuiSetSuiUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetSuiData(void);
PtrT_TransactionCheckResult GuiGetSuiCheckResult(void);
void FreeSuiMemory(void);
int GetSuiDetailLen(void *param);
void GetSuiDetail(void *indata, void *param, uint32_t maxLen);
UREncodeResult *GuiGetSuiSignQrCodeData(void);

#endif
