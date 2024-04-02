#ifndef _GUI_MULTI_SIG_IMPORT_WALLET_INFO_WIDGETS_H
#define _GUI_MULTI_SIG_IMPORT_WALLET_INFO_WIDGETS_H

void GuiSetMultisigImportWalletData(URParseResult *urResult, URParseMultiResult *multiResult, bool multi);

void GuiImportMultisigWalletInfoWidgetsInit();
void GuiImportMultisigWalletInfoWidgetsDeInit();
void GuiImportMultisigWalletInfoWidgetsRefresh();
void GuiImportMultisigWalletInfoWidgetsRestart();
void GuiImportMultisigWalletInfoVerifyPasswordSuccess(void);

#endif
