#ifndef _GUI_TRANSACTION_DETAIL_WIDGETS_H
#define _GUI_TRANSACTION_DETAIL_WIDGETS_H

void GuiTransactionDetailInit(uint8_t viewType);
void GuiTransactionDetailDeInit(void);
void GuiTransactionDetailRefresh(void);
void GuiTransactionDetailVerifyPasswordSuccess(void);
void GuiSignVerifyPasswordErrorCount(void *param);
void GuiSignDealFingerRecognize(void *param);
void GuiTransactionDetailParseSuccess(void *param);
void GuiClearQrcodeSignCnt(void);

#endif /* _GUI_TRANSACTION_DETAIL_WIDGETS_H */