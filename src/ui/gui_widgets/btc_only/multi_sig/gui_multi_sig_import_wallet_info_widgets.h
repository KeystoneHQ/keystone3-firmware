#ifndef _GUI_MULTI_SIG_IMPORT_WALLET_INFO_WIDGETS_H
#define _GUI_MULTI_SIG_IMPORT_WALLET_INFO_WIDGETS_H

void GuiSetMultisigImportWalletDataByQRCode(URParseResult *urResult, URParseMultiResult *multiResult, bool multi);
void GuiSetMultisigImportWalletDataBySDCard(char* walletConfig);

void GuiImportMultisigWalletInfoWidgetsInit();
void GuiImportMultisigWalletInfoWidgetsDeInit();
void GuiImportMultisigWalletInfoWidgetsRefresh();
void GuiImportMultisigWalletInfoWidgetsRestart();
void GuiImportMultisigWalletInfoVerifyPasswordSuccess(void);

#endif
