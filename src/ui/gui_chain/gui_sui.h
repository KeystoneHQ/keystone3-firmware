#ifndef _GUI_SUI_H
#define _GUI_SUI_H

void GuiSetSuiUrData(void *data, bool multi);
void *GuiGetSuiData(void);
PtrT_TransactionCheckResult GuiGetSuiCheckResult(void);
void FreeSuiMemory(void);
int GetSuiDetailLen(void *param);
void GetSuiDetail(void *indata, void *param);
UREncodeResult *GuiGetSuiSignQrCodeData(void);

#endif
