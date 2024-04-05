#ifndef _GUI_TRANSACTION_DETAIL_WIDGETS_H
#define _GUI_TRANSACTION_DETAIL_WIDGETS_H

typedef enum {
    //for all single sig transactions.
    TRANSACTION_TYPE_NORMAL,
    //for btc multisig transaction, with different destination.
    TRANSACTION_TYPE_BTC_MULTISIG,
    
    TRANSACTION_TYPE_BUTT,
} TransactionType;

void GuiTransactionDetailInit(uint8_t viewType);
void GuiTransactionDetailDeInit(void);
void GuiTransactionDetailRefresh(void);
void GuiTransactionDetailVerifyPasswordSuccess(void);
void GuiSignVerifyPasswordErrorCount(void *param);
void GuiSignDealFingerRecognize(void *param);
void GuiTransactionDetailParseSuccess(void *param);
void GuiClearQrcodeSignCnt(void);
void GuiTransactionParseFailed(void);
void GuiSetCurrentTransactionType(TransactionType);
TransactionType GuiGetCurrentTransactionType();

#endif /* _GUI_TRANSACTION_DETAIL_WIDGETS_H */