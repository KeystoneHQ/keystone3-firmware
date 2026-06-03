#ifndef _GUI_DERIVE_CONTEXT_HASH_REQUEST_WIDGETS_H_
#define _GUI_DERIVE_CONTEXT_HASH_REQUEST_WIDGETS_H_

#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

void GuiDeriveContextHashRequestInit(bool isUsb);
void GuiDeriveContextHashRequestDeInit(void);
void GuiDeriveContextHashRequestRefresh(void);
void GuiDeriveContextHashWidgetHandleURGenerate(char *data, uint16_t len);
void GuiDeriveContextHashWidgetHandleURGenerateFail(char *message);
void GuiDeriveContextHashWidgetHandleURUpdate(char *data, uint16_t len);
void GuiSetDeriveContextHashRequestData(void *urResult, void *multiResult, bool is_multi);
void GuiDeriveContextHashPasswordErrorCount(void *param);
void GuiDeriveContextHashUsbPullout(void);
void DeriveContextHashHiddenKeyboardAndShowAnimateQR(void);

#endif
