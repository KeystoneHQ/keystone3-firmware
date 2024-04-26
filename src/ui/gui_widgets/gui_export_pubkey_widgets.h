#ifndef _GUI_EXPORT_PUBKEY_WIDGETS_H
#define _GUI_EXPORT_PUBKEY_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"

void GuiExportPubkeyInit(uint8_t chain);
void GuiExportPubkeyDeInit(void);
void OpenExportViewHandler(lv_event_t *e);
void OpenExportMultisigViewHandler(lv_event_t *e);

#endif
