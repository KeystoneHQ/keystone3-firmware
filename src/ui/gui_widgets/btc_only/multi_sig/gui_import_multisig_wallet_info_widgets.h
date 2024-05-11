#ifndef _GUI_MULTI_SIG_IMPORT_WALLET_INFO_WIDGETS_H
#define _GUI_MULTI_SIG_IMPORT_WALLET_INFO_WIDGETS_H
#include "librust_c.h"
#include <stdbool.h>
#include <stdint.h>
void GuiSetMultisigImportWalletDataByQRCode(URParseResult *urResult,
        URParseMultiResult *multiResult,
        bool multi);
uint32_t GuiSetMultisigImportWalletDataBySDCard(char *walletConfig);

void GuiImportMultisigWalletInfoWidgetsInit(void);
void GuiImportMultisigWalletInfoWidgetsDeInit(void);
void GuiImportMultisigWalletInfoVerifyPasswordSuccess(void);
void GuiImportMultisigPasswordErrorCount(void *param);

#endif
