#ifndef _GUI_SCAN_WIDGETS_H
#define _GUI_SCAN_WIDGETS_H

#include "rust.h"

void GuiScanInit(void *param, uint16_t len);
void GuiScanDeInit();
void GuiScanRefresh();
void GuiScanResult(bool result, void *param);
void GuiTransactionCheckPass(void);
void GuiTransactionCheckFailed(PtrT_TransactionCheckResult result);

#endif /* _GUI_SCAN_WIDGETS_H */
