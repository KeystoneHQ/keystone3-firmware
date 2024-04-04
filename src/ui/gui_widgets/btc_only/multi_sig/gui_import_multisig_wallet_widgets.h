#ifndef _GUI_MULTI_SIG_IMPORT_WALLET_SUCCESS_WIDGETS_H
#define _GUI_MULTI_SIG_IMPORT_WALLET_SUCCESS_WIDGETS_H

void GuiImportMultisigWalletWidgetsInit(char* verifyCode);
void GuiImportMultisigWalletWidgetsDeInit();
void GuiImportMultisigWalletWidgetsRefresh();
void GuiImportMultisigWalletSuccessWidgetsRestart();
int8_t GuiImportMultiNextTile(void);
int8_t GuiImportMultiPrevTile(void);

#endif
