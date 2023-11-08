#ifndef _GUI_ANIMATING_QRCODE_H
#define _GUI_ANIMATING_QRCODE_H

#define QR_SIZE_FULL 420
#define QR_SIZE_REGULAR 294
#define QR_BG_COLOR WHITE_COLOR
#define QR_FG_COLOR BLACK_COLOR
#define TIMER_UPDATE_INTERVAL 200
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 800

#include "rust.h"
#include "lvgl.h"

typedef UREncodeResult *(*GenerateUR)(void);

void GuiAnimatingQRCodeInit(lv_obj_t* parent, GenerateUR dataFunc, bool showPending);
void GuiAnimatingQRCodeInitWithCustomSize(lv_obj_t* parent, GenerateUR dataFunc, bool showPending, uint16_t w, uint16_t h);
void GuiAnimatingQRCodeControl(bool pause);
void GuiAnimantingQRCodeFirstUpdate(char* data, uint16_t len);
void GuiAnimatingQRCodeUpdate(char* data, uint16_t len);
void GuiAnimatingQRCodeDestroyTimer();

#endif