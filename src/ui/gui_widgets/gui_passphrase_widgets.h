/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Passphrase input UI widgets.
 * Author: leon sun
 * Create: 2023-6-8
 ************************************************************************************************/

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

