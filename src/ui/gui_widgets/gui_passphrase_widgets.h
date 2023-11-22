#ifndef _GUI_PASSPHRASE_WIDGETS_H
#define _GUI_PASSPHRASE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"


void GuiPassphraseInit(void);
void GuiPassphraseDeInit(void);
void GuiPassphraseRefresh(void);
void GuiPassphrasePrevTile(void);
void GuiPassphraseDone(void);
bool GuiPassphraseLoadingIsTop(void);
#endif

