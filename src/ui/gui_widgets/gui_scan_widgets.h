#ifndef _GUI_SCAN_WIDGETS_H
#define _GUI_SCAN_WIDGETS_H

#include "rust.h"

#ifdef BTC_ONLY
//todo:  add types as needed
typedef enum {
    OTHER_PAGE = 0,
    IMPORT_MULTI_SIG_WALLET_PAGE
} FromPageEnum;
#endif

void GuiScanInit();
void GuiScanDeInit();
void GuiScanRefresh();
void GuiScanResult(bool result, void *param);
void GuiTransactionCheckPass(void);
void GuiTransactionCheckFailed(PtrT_TransactionCheckResult result);
void GuiSetScanViewTypeFiler(ViewType *viewType, int number);

#ifdef BTC_ONLY
void SelectMicroCardFile(void);
void GuiScanSetFromPage(FromPageEnum fromPage);
#endif

#endif /* _GUI_SCAN_WIDGETS_H */
