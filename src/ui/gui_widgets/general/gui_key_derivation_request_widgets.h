#ifndef _GUI_KEY_DERIVATION_REQUEST_WIDGETS_H_
#define _GUI_KEY_DERIVATION_REQUEST_WIDGETS_H_

#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

void GuiKeyDerivationRequestInit(bool isUsb);
void GuiKeyDerivationRequestDeInit();
void GuiKeyDerivationRequestRefresh();
void GuiKeyDerivationWidgetHandleURGenerate(char *data, uint16_t len);
void GuiKeyDerivationWidgetHandleURUpdate(char *data, uint16_t len);
void GuiSetKeyDerivationRequestData(void *data, void *multiResult, bool is_multi);
void GuiResetCurrentKeyDerivationCache(uint8_t index);
void GuiKeyDerivePasswordErrorCount(void *param);

void HiddenKeyboardAndShowAnimateQR();
#endif