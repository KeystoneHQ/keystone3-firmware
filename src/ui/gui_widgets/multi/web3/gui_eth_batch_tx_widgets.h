#ifndef _GUI_ETH_BATCH_TX_WIDGETS_H
#define _GUI_ETH_BATCH_TX_WIDGETS_H

#include "rust.h"

#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"


void GuiEthBatchTxWidgetsInit();
void GuiEthBatchTxWidgetsDeInit();
void GuiEthBatchTxWidgetsRefresh();
void GuiEthBatchTxWidgetsVerifyPasswordSuccess();
void GuiEthBatchTxWidgetsTransactionParseSuccess();
void GuiEthBatchTxWidgetsTransactionParseFail();
void GuiCreateEthBatchTxWidget();
void GuiSetEthBatchTxData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);

void GuiEthBatchTxWidgetsSignVerifyPasswordErrorCount(void *param);
UREncodeResult *GuiGetEthBatchTxSignQrCodeData();

#endif
