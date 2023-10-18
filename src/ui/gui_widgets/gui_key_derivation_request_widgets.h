#ifndef _GUI_KEY_DERIVATION_REQUEST_WIDGETS_H_
#define _GUI_KEY_DERIVATION_REQUEST_WIDGETS_H_

#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

void GuiKeyDerivationRequestInit();
void GuiKeyDerivationRequestDeInit();
void GuiKeyDerivationRequestRefresh();
void GuiKeyDerivationRequestNextTile();
void GuiKeyDerivationRequestPrevTile();
void GuiSetKeyDerivationRequestData(void *data, bool is_multi);


#endif