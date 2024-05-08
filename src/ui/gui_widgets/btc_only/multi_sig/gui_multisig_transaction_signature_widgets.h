#ifndef _GUI_MULTISIG_TRANSACTION_SIGNATURE_WIDGETS_H
#define _GUI_MULTISIG_TRANSACTION_SIGNATURE_WIDGETS_H

#include "stdbool.h"
#include "stdint.h"

void GuiMultisigTransactionSignaureWidgetsInit();
void GuiMultisigTransactionSignaureWidgetsDeInit();
void GuiMultisigTransactionSignaureWidgetsRefresh();
void GuiMultisigTransactionSignatureSetSignStatus(char *signStatus, bool signCompleted, uint8_t *psbtHex, uint32_t psbtLen);
void GuiMultisigTransactionSginatureSetPsbtName(char *psbtName);

#endif
