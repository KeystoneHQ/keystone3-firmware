#ifndef _GUI_MULTISIG_READ_SDCARD_WIDGETS_H_
#define _GUI_MULTISIG_READ_SDCARD_WIDGETS_H_
#include "gui.h"

typedef enum {
    ALL = 0,
    ONLY_TXT,
    ONLY_PSBT,
    ONLY_JSON,
} FileFilterType;

void GuiMultisigReadSdcardWidgetsInit(uint8_t fileFilterType);
void GuiMultisigReadSdcardWidgetsDeInit();

#endif