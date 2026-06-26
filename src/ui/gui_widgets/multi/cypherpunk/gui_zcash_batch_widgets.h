#ifndef _GUI_ZCASH_BATCH_WIDGETS_H
#define _GUI_ZCASH_BATCH_WIDGETS_H

#include "gui.h"
#include "rust.h"

void GuiSetZcashBatchUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void GuiZcashBatchWidgetsInit(void);
void GuiZcashBatchWidgetsDeInit(void);
void GuiZcashBatchWidgetsRefresh(void);
void GuiZcashBatchWidgetsVerifyPasswordSuccess(void);
void GuiZcashBatchWidgetsSignVerifyPasswordErrorCount(void *param);
void GuiZcashBatchWidgetsUsbPullout(void);
void GuiZcashBatchWidgetsTransactionParseSuccess(void);
void GuiZcashBatchWidgetsTransactionParseFail(void);
UREncodeResult *GuiGetZcashBatchSignQrCodeData(void);
UREncodeResult *GuiGetZcashBatchSignUrDataUnlimited(void);
#ifdef CYPHERPUNK_VERSION
PtrT_TransactionCheckResult GuiGetZcashBatchCheckResult(void);
#endif

#endif
