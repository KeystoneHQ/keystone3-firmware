#ifndef _GUI_SCAN_WIDGETS_H
#define _GUI_SCAN_WIDGETS_H

#include "rust.h"
#include "qrdecode_task.h"

#ifdef CYPHERPUNK_VERSION
typedef struct {
    UrViewType_t viewType;
    char address[1025];
} ZcashAddressScanResult_t;
#endif

void GuiScanInit(void *param, uint16_t len);
void GuiScanDeInit();
void GuiScanRefresh();
void GuiScanResult(bool result, void *param);
void GuiTransactionCheckPass(void);
#ifdef WEB3_VERSION
void GuiArTransactionReady(const char *address);
#endif
void GuiTransactionCheckFailed(PtrT_TransactionCheckResult result);

#endif /* _GUI_SCAN_WIDGETS_H */
