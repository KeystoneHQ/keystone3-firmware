#ifndef _GUI_MULTI_SIG_IMPORT_WALLET_SUCCESS_WIDGETS_H
#define _GUI_MULTI_SIG_IMPORT_WALLET_SUCCESS_WIDGETS_H

void GuiImportMultisigWalletWidgetsInit(char* verifyCode, uint16_t len);
void GuiImportMultisigWalletWidgetsDeInit();
void GuiImportMultisigWalletWidgetsRefresh();
void GuiImportMultisigWalletSuccessWidgetsRestart();
void GuiImportMultisigWalletInfoVerifyPasswordSuccess();
int8_t GuiImportMultiNextTile(void);
int8_t GuiImportMultiPrevTile(void);
void GuiSetExportMultiSigSwitch(void);
void ModelGenerateMultiSigAddress(char *address, uint32_t maxLen, char *walletConfig, uint32_t index);

#endif
