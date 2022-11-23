#ifndef _GUI_ANIMATING_QRCODE_H
#define _GUI_ANIMATING_QRCODE_H

#include "rust.h"

typedef UREncodeResult *(*GenerateUR)(void);

void GuiAnimatingQRCodeInit(lv_obj_t* parent, GenerateUR dataFunc, bool showPending);
void GuiAnimatingQRCodeControl(bool pause);
void GuiAnimantingQRCodeFirstUpdate(char* data, uint16_t len);
void GuiAnimatingQRCodeUpdate(char* data, uint16_t len);
void GuiAnimatingQRCodeDestroyTimer();

#endif