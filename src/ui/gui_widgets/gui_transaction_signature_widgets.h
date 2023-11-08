#ifndef _GUI_TRANSACTION_DETAIL_WIDGETS_H
#define _GUI_TRANSACTION_DETAIL_WIDGETS_H

void GuiTransactionSignatureInit(uint8_t viewType);
void GuiTransactionSignatureDeInit(void);
void GuiTransactionSignatureRefresh(void);
void GuiTransactionSignatureHandleURGenerate(char *data, uint16_t len);
void GuiTransactionSignatureHandleURUpdate(char *data, uint16_t len);
#endif /* _GUI_TRANSACTION_WIDGETS_H */